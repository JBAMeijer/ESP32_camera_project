#include "pc_interface.h"

#include <stdint.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>

#define MAX 80
#define PORT 8080
#define SA struct sockaddr
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

typedef enum
{
    WAIT_FOR_HEADER,
    WAIT_FOR_DATA
} rx_state_t;

void pc_wifi_interface_rx();
void pc_wifi_interface_process_rx_complete(void);
void pc_wifi_interface_write(eFrameType, uint8_t*, uint32_t);

static bool _connected = false;
static struct pollfd pfds[2];
static int sockfd, connfd, len, new_socket;
static int status;
static struct sockaddr_in servaddr, cli;
static int opt = 1;
static socklen_t addrlen = sizeof(servaddr);

static uint32_t rx_cnt;
static rx_state_t rx_state;
static eFrameType frame_type;
static uint32_t rx_expected_len;
static bool rx_complete;
static uint8_t rx_buffer[2048] = {0};

#define PC_IS_READY_FOR_IMG_TITLE  (0x00000001)
#define PC_IS_READY_FOR_IMG_STRUCT (0x00000002)
#define PC_IS_READY_FOR_IMG_DATA   (0x00000004)
#define PC_IS_READY_FOR_BENCHMARK  (0x00000008)

static uint32_t pc_is_ready = 0;

int pc_wifi_interface_start() {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        printf("socket creation failed...\n");
        return(-1);
    } else {
        printf("Socket succesfully created..\n");
    }
    
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    
    if(inet_pton(AF_INET, "192.168.4.1", &servaddr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return(-1);
    }
    
    printf("Binding:\n");
    if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		perror("bind failed");
		return(-1);
	}
	printf("Listening:\n");
	if(listen(sockfd, 3) < 0) {
		perror("listen");
		return(-1);
	}
	printf("Accepting:\n");
	if(poll(pfds, 2, 20) > 0) {
		if(pfds[0].revents & (POLLERR)) {
            printf("Socket error\n");
            return(-1);
        }
        if(pfds[0].revents & POLLIN){
			if((new_socket = accept(sockfd, (struct sockaddr*)&servaddr, &addrlen)) < 0) {
				perror("accept\n");
				return(-1);
			}
			
			pfds[1].fd = new_socket;
			pfds[1].events = POLLIN;
			
			_connected = true;
			printf("Connected\n");
		}
	} else {
		printf("No incomming connection\n");
	}
    
    return(0);
}

void pc_wifi_interface_stop() {
	close(new_socket);
	close(sockfd);
}

void close_client_socket() {
	printf("Connection closed\n");
	close(new_socket);
	pfds[1].fd = -1;
	pfds[1].events = 0;
	pc_is_ready = 0;
	_connected = false;
}

void pc_wifi_interface_rx() {
    if(poll(pfds, 2, 5) > 0) {
        if(pfds[0].revents & (POLLERR | POLLHUP)) {
            printf("Socket error\n");
            return;
        }

        if(pfds[0].revents & POLLIN) {
			if(!_connected) {
				if((new_socket = accept(sockfd, (struct sockaddr*)&servaddr, &addrlen)) < 0) {
					perror("accept\n");
				}
				
				pfds[1].fd = new_socket;
				pfds[1].events = POLLIN;
				_connected = true;
				printf("Connected\n");
			}
        }
        
        if(pfds[1].revents & POLLIN) {
			if(_connected) {
				uint32_t amount_of_bytes = read(new_socket, rx_buffer, 2048);
				if(amount_of_bytes == 0) close_client_socket();
				else {
					rx_cnt += amount_of_bytes;

					if(rx_state == WAIT_FOR_HEADER && rx_cnt >= 5) {
						rx_expected_len = 0;

						// retrieve data
						rx_expected_len |= (uint32_t)rx_buffer[1];
						rx_expected_len |= (uint32_t)rx_buffer[2] << 8;
						rx_expected_len |= (uint32_t)rx_buffer[3] << 16;
						rx_expected_len |= (uint32_t)rx_buffer[4] << 24;

						if(rx_expected_len == 0) {
							// All data is here
							rx_complete = true;
							rx_state = WAIT_FOR_HEADER;
						} else {
							// More data expected
							if((rx_cnt - 5) == rx_expected_len) {
								rx_complete = true;
								rx_state = WAIT_FOR_HEADER;
							} else {
								rx_state = WAIT_FOR_DATA;
							}
						}
					}

					if((rx_state == WAIT_FOR_DATA) && ((rx_cnt - 5) == rx_expected_len)) {
						rx_complete = true;
						rx_state = WAIT_FOR_HEADER;
					}
					printf("Received %d bytes\n", amount_of_bytes);
				}
			}
        }
    }
}


void pc_wifi_interface_process_rx_complete() {
        // Don't actually process anything until reception is complete
    if(rx_complete == false)
        return;

    rx_complete = false;
    rx_expected_len = 0;
    rx_cnt = 0;

    switch (rx_buffer[0])
    {
        case KEEP_ALIVE_REQ:
        {
            printf("Sending keep alive\n");
            pc_wifi_interface_write(KEEP_ALIVE_ACK, NULL, 0);
        }
        break;
        case VERSION_REQ:
        {
            // Send version
            printf("Sending version\n");
            uint8_t buffer[3] = {0, 0, 1};
            pc_wifi_interface_write(VERSION_ACK, buffer, 3);
        }
        break;
        case IMAGE_TITLE_REQ:
        {
            // Set flag
            pc_is_ready |= PC_IS_READY_FOR_IMG_TITLE;
        }
        break;
        case IMAGE_STRUCT_REQ:
        {
            // Set flag
            pc_is_ready |= PC_IS_READY_FOR_IMG_STRUCT;
        }
        break;
        case IMAGE_DATA_REQ:
        {
            // Set flag
            pc_is_ready |= PC_IS_READY_FOR_IMG_DATA;
        }
        break;
        case BENCHMARK_REQ:
        {
            // Set flag
            pc_is_ready |= PC_IS_READY_FOR_BENCHMARK;
        }
        break;
        case IMAGE_TITLE_OR_BENCHMARK_REQ:
        {
            // Set flags
            pc_is_ready |= (PC_IS_READY_FOR_IMG_TITLE | PC_IS_READY_FOR_BENCHMARK);
        }
        break;
    
    default:
        break;
    }
}

void pc_wifi_interface_update() {
	pc_wifi_interface_rx(); 
    if(rx_complete) pc_wifi_interface_process_rx_complete();
}

void pc_wifi_interface_write(eFrameType type, uint8_t* buffer, uint32_t len) {
    uint8_t tmp[5];
    uint8_t *d = (uint8_t *)(&len);

    //Set header
    tmp[0] = type;
    tmp[1] = *d++;
    tmp[2] = *d++;
    tmp[3] = *d++;
    tmp[4] = *d++;

	uint32_t bytes_send = send(new_socket, tmp, 5, 0);
    if(bytes_send != 5)
        return;

    if(len > 0) {
		send(new_socket, buffer, len, 0);
    }
}

void pc_wifi_interface_send_benchmark(benchmark_t* benchmark)
{
    //printf("Device wants to send benchmark\n");
    //printf("pc_is_ready byte: %d\n", pc_is_ready);
    while(!(pc_is_ready & PC_IS_READY_FOR_BENCHMARK) && _connected)
    {
        pc_wifi_interface_process_rx_complete();
        pc_wifi_interface_rx();
    }

    if(!_connected)
        return;

    pc_is_ready = 0;

    pc_wifi_interface_write(BENCHMARK_ACK, (uint8_t*)benchmark, sizeof(benchmark_t));
    //printf("Benchmark send\n");
}

