#include "device_wifi_interface.h"

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

#include "raylib.h"
#include "operators.h"

#define MAX 80
#define PORT 8080
#define SA struct sockaddr
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#define START_BUFFER_SIZE (320*240*2)

const u8 DEVICE_VERSION_RQ_FRAME[] = { VERSION_REQ, 0, 0, 0, 0 };
const u8 KEEP_ALIVE_RQ_FRAME[] = { KEEP_ALIVE_REQ, 0, 0, 0, 0 };
const u8 BENCHMARK_RQ_FRAME[] = { BENCHMARK_REQ, 0, 0, 0, 0 };

const u8 IMAGE_TITLE_RQ_FRAME[] = { IMAGE_TITLE_REQ, 0, 0, 0, 0 };
const u8 IMAGE_STRUCT_RQ_FRAME[] = { IMAGE_STRUCT_REQ, 0, 0, 0, 0 };
const u8 IMAGE_DATA_RQ_FRAME[] = { IMAGE_DATA_REQ, 0, 0, 0, 0 };
const u8 IMAGE_TITLE_OR_BENCHMARK_RQ_FRAME[] = { IMAGE_TITLE_OR_BENCHMARK_REQ, 0, 0, 0, 0 };

static bool _connected = false;
static struct pollfd pfds[1];
static int sockfd, connfd, len;
static int status;
static struct sockaddr_in servaddr, cli;

static u32 rx_cnt;
static rx_state_t rx_state;
static eFrameType frame_type;
static u32 rx_expected_len;
static bool rx_complete;
static u8 rx_buffer[4096] = {0};
static u32 data_buffer_len = 0;
static u8* data_buffer = NULL;

static eDeviceDataState benchmark = IDLE;

static image_t image_struct;

deviceVersionFuncDef _device_version_func = NULL;
benchmarkFuncDef _benchmark_func = NULL;
imageFuncDef _image_func = NULL;

void handle_received_image_title(void);
void handle_received_image_struct(void);
void handle_received_image_data(void);
void send_req_for_new_benchmark_or_image_title(void);
void send_req_for_image_struct(void);
void send_req_for_image_data(void);

s32 open_connection(deviceVersionFuncDef device_version_func, benchmarkFuncDef benchmark_func, imageFuncDef image_func) {
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

    status = connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(status == 0) {
        printf("Succesfully connected...\n");
    } else {
        printf("Failed to connect to device server...\n");
        return(-1);
    }

    int enableKeepAlive = 1;
    int count = 3;      // number of emergency requests
    int maxIdle = 1;    // delay (s) between requests, max idle time
    int interval = 1;   // delay (s) between emergency requests. "count" request are send

    setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &enableKeepAlive, sizeof(enableKeepAlive));
    setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &maxIdle, sizeof(maxIdle));
    setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count));
    setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
    _connected = true;

    _device_version_func = device_version_func;
    _benchmark_func = benchmark_func;
    _image_func = image_func;

    if(data_buffer == NULL) {
        data_buffer = malloc(START_BUFFER_SIZE);
        data_buffer_len = START_BUFFER_SIZE;
    }

    req_device_version();

    return(0);
}

void close_connection(void) {
    close(sockfd);
    _connected = false;
    rx_complete = false;
    rx_expected_len = 0;
    rx_cnt = 0;
}

void pc_wifi_interface_rx() {
    if(poll(pfds, 1, 20) > 0) {
        if(pfds[0].revents & (POLLERR | POLLHUP)) {
            printf("Socket was closed\n");
            close_connection();
            return;
        }

        if(pfds[0].revents & POLLIN) {
            u32 amount_of_bytes = read(sockfd, rx_buffer, 4096);
            //printf("Received %d bytes\n", amount_of_bytes);

            rx_cnt += amount_of_bytes;
            //printf("rx_cnt %d\n", rx_cnt);
            if(rx_state == WAIT_FOR_HEADER && rx_cnt >= 5) {
                // retrieve data
                frame_type = rx_buffer[0];
                rx_expected_len |= (uint32_t)rx_buffer[1];
                rx_expected_len |= (uint32_t)rx_buffer[2] << 8;
                rx_expected_len |= (uint32_t)rx_buffer[3] << 16;
                rx_expected_len |= (uint32_t)rx_buffer[4] << 24;

                printf("Expected length data to receive: %d\n", rx_expected_len);

                if(rx_expected_len == 0) {
                    // All data is here
                    rx_complete = true;
                    rx_state = WAIT_FOR_HEADER;
                } else {
                    // More data expected
                    if(rx_expected_len > data_buffer_len) {
                        MemFree(data_buffer);
                        data_buffer = MemAlloc(rx_expected_len);
                        data_buffer_len = rx_expected_len;
                    }
                    //data_buffer = MemAlloc(rx_expected_len*sizeof(u8));
                    if((rx_cnt - 5) == rx_expected_len) {
                        for(u32 i = 0; i < rx_expected_len; i++) {
                            data_buffer[i] = rx_buffer[i + 5];
                        }
                        rx_complete = true;
                        rx_state = WAIT_FOR_HEADER;
                    } else {
                        rx_state = WAIT_FOR_DATA;
                        u32 buffer_index = 5;
                        for(u32 i = rx_cnt - amount_of_bytes; i < amount_of_bytes - 5; i++) {
                            data_buffer[i] = rx_buffer[buffer_index];
                            buffer_index++;
                        }
                    }
                }
            } else {
                if((rx_state == WAIT_FOR_DATA) && ((rx_cnt - 5) == rx_expected_len)) {
                    u32 buffer_index = 0;

                    for(u32 i = rx_cnt - amount_of_bytes - 5; i < rx_cnt - 5; i++) {
                        data_buffer[i] = rx_buffer[buffer_index];
                        buffer_index++;
                    }
                    rx_complete = true;
                    rx_state = WAIT_FOR_HEADER;
                } else if((rx_state == WAIT_FOR_DATA) && ((rx_cnt - 5) != rx_expected_len)) {
                    u32 buffer_index = 0;
                    for(u32 i = rx_cnt - amount_of_bytes - 5; i < rx_cnt - 5; i++) {
                        data_buffer[i] = rx_buffer[buffer_index];
                        buffer_index++;
                    }
                }
            }
        }
    }
}

void pc_wifi_interface_process_rx_complete(void) {
    rx_complete = false;
    rx_expected_len = 0;
    rx_cnt = 0;

    switch (frame_type)
    {
        case KEEP_ALIVE_ACK:
        {
            printf("Keep alive received\n");
        }
        break;
        case VERSION_ACK:
        {
            printf("Device version\n");
            if(_device_version_func) _device_version_func(data_buffer[0], data_buffer[1], data_buffer[2]);
            send_req_for_new_benchmark_or_image_title();
        }
        break;
        case BENCHMARK_ACK:
        {
            handle_received_benchmark_data();
            send_req_for_new_benchmark_or_image_title();
        }
        break;
        case IMAGE_TITLE_ACK:
        {
            handle_received_image_title();
            send_req_for_image_struct();
        }
        break;
        case IMAGE_STRUCT_ACK:
        {
            handle_received_image_struct();
            send_req_for_image_data();
        }
        break;
        case IMAGE_DATA_ACK:
        {
            handle_received_image_data();
            send_req_for_new_benchmark_or_image_title();
        }
        break;
    
    default:
        break;
    }
}

void send_keepalive(void) {
    if(send(sockfd, KEEP_ALIVE_RQ_FRAME, sizeof(KEEP_ALIVE_RQ_FRAME), 0) >= 0) {
        printf("Succesfully send keep alive req\n");
    } else {
        printf("sending of keep alive failed\n");
    }
}

void req_device_version(void) {
    if(send(sockfd, DEVICE_VERSION_RQ_FRAME, sizeof(DEVICE_VERSION_RQ_FRAME), 0) >= 0) {
        printf("Succesfully send version rq\n");
    } else {
        printf("sending of device version request failed\n");
    }
}

void send_req_for_new_benchmark(void) {
    if(send(sockfd, BENCHMARK_RQ_FRAME, sizeof(BENCHMARK_RQ_FRAME), 0) >= 0) {
        printf("Succesfully send benchmark req\n");
    } else {
        printf("sending of benchmark request failed\n");
    }
}

void handle_received_benchmark_data(void) {
    benchmark_t benchmark = create_benchmark_from_data(data_buffer);
    if(_benchmark_func) _benchmark_func(benchmark);
}

bool connected(void) {
    return _connected;
}

void device_wifi_interface_update(void) {
    if(!_connected) return;

    pc_wifi_interface_rx(); 
    if(rx_complete) pc_wifi_interface_process_rx_complete();
}

void handle_received_image_title(void) {
    printf("Received image title: %s\n", data_buffer);
}

void handle_received_image_struct(void) {
    u32 cols = 0, rows = 0;
    eImageView view = 0;
    eImageType type = 0;
    cols |= (u32)data_buffer[0];
    cols |= (u32)data_buffer[1] << 8;
    cols |= (u32)data_buffer[2] << 16;
    cols |= (u32)data_buffer[3] << 24;
    
    rows |= (u32)data_buffer[4];
    rows |= (u32)data_buffer[5] << 8;
    rows |= (u32)data_buffer[6] << 16;
    rows |= (u32)data_buffer[7] << 24;

    view |= (u32)data_buffer[8];
    view |= (u32)data_buffer[9] << 8;
    view |= (u32)data_buffer[10] << 16;
    view |= (u32)data_buffer[11] << 24;

    type |= (u32)data_buffer[12];
    type |= (u32)data_buffer[13] << 8;
    type |= (u32)data_buffer[14] << 16;
    type |= (u32)data_buffer[15] << 24;

    image_struct.cols = cols;
    image_struct.rows = rows;
    image_struct.view = view;
    image_struct.type = type;
    image_struct.data = NULL;

    printf("Image - Cols: %d, Rows: %d, View: %d, Type: %d\n", image_struct.cols, image_struct.rows, image_struct.view, image_struct.type);
}

void handle_received_image_data(void) {
    image_struct.data = data_buffer;
    if(_image_func) _image_func(&image_struct);
    memset(data_buffer, 0, data_buffer_len);
}

void send_req_for_new_benchmark_or_image_title(void) {
    if(send(sockfd, IMAGE_TITLE_OR_BENCHMARK_RQ_FRAME, sizeof(IMAGE_TITLE_OR_BENCHMARK_RQ_FRAME), 0) >= 0) {
        printf("Succesfully send benchmark or image title req\n");
    } else {
        printf("sending of benchmark or image title request failed\n");
    }
}

void send_req_for_image_struct(void) {
    if(send(sockfd, IMAGE_STRUCT_RQ_FRAME, sizeof(IMAGE_STRUCT_RQ_FRAME), 0) >= 0) {
        printf("Succesfully send image struct req\n");
    } else {
        printf("sending of image struct request failed\n");
    }
}

void send_req_for_image_data(void) {
    if(send(sockfd, IMAGE_DATA_RQ_FRAME, sizeof(IMAGE_DATA_RQ_FRAME), 0) >= 0) {
        printf("Succesfully send image data req\n");
    } else {
        printf("sending of image data request failed\n");
    }
}