#include "pc_interface.h"

typedef enum
{
    WAIT_FOR_HEADER,
    WAIT_FOR_DATA
} rx_state_t;

eDeviceMode device_mode_state = MODE_DISCONNECTED;

WiFiServer server(8080);
WiFiClient wifi_client;

static uint8_t rx_buffer[64];
static uint32_t rx_cnt = 0;
static rx_state_t rx_state;
static bool rx_complete;
static uint32_t rx_expected_len = 0;

#define PC_IS_READY_FOR_IMG_TITLE  (0x00000001)
#define PC_IS_READY_FOR_IMG_STRUCT (0x00000002)
#define PC_IS_READY_FOR_IMG_DATA   (0x00000004)
#define PC_IS_READY_FOR_BENCHMARK  (0x00000008)

static uint32_t pc_is_ready = 0;

void pc_wifi_interface_rx();
void pc_wifi_interface_process_rx_complete();

void pc_wifi_interface_start(const char* SSID, const char* password)
{
    Serial.println("Configure acces point..."); 
    WiFi.softAP(SSID, password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.begin();
    Serial.println("Server started");
    pc_wifi_interface_reset();
}

void pc_wifi_interface_reset()
{
    rx_complete = false;
    pc_is_ready = false;
    rx_cnt = 0;
    rx_state = WAIT_FOR_HEADER;

    device_mode_state = MODE_DISCONNECTED;
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

    uint32_t bytes_send = wifi_client.write(tmp, 5);
    if(bytes_send != 5)
        return;

    //delay(50); // Give the system some time to send the data.

    if(len > 0) {
        wifi_client.write(buffer, len);
        // register uint32_t cnt = len;
        // register uint32_t n;
        // register uint32_t index = 0;

        // while((cnt > 0) && (wifi_client.connected())) {
        //     if(cnt > 512) n = 512;
        //     else n = cnt;

        //     bytes_send = wifi_client.write(&buffer[index], n);
        //     if(bytes_send != n)
        //         return;
            
        //     index += n;
        //     cnt -= n;
        // }
    }
}

void pc_wifi_interface_update()
{
    if(!wifi_client.connected() && device_mode_state == MODE_CONNECTED) {
        device_mode_state = MODE_DISCONNECTED;
        Serial.println("Device mode changed to: Disconnected");
        digitalWrite(BUILTIN_LED, HIGH);
        pc_is_ready = 0;
    }

    if(!wifi_client.connected()){
        WiFiClient temp_client = server.available();
        if(temp_client.connected()) {
            wifi_client = temp_client;
            device_mode_state = MODE_CONNECTED;
            Serial.println("Device mode changed to: Connected");
            digitalWrite(BUILTIN_LED, LOW);
        }
    }

    if(wifi_client.connected()){
        pc_wifi_interface_rx();
        pc_wifi_interface_process_rx_complete();
    }
}

void pc_wifi_interface_rx() {
    uint8_t amount_of_bytes = wifi_client.available();
    if(amount_of_bytes > 64) {
        amount_of_bytes = 64;
    }

    wifi_client.readBytes(rx_buffer, amount_of_bytes);
    
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
            Serial.println("Sending keep alive");
            pc_wifi_interface_write(KEEP_ALIVE_ACK, NULL, 0);
        }
        break;
        case VERSION_REQ:
        {
            // Set version
            Serial.println("Sending version");
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

void pc_wifi_interface_send_benchmark(benchmark_t* benchmark)
{
    Serial.println("Device wants to send benchmark");
    Serial.println(pc_is_ready, HEX);
    while(!(pc_is_ready & PC_IS_READY_FOR_BENCHMARK) && wifi_client.connected())
    {
        pc_wifi_interface_process_rx_complete();
        pc_wifi_interface_rx();
    }

    if(!wifi_client.connected())
        return;

    pc_is_ready = 0;

    pc_wifi_interface_write(BENCHMARK_ACK, (uint8_t*)benchmark, sizeof(benchmark_t));
    Serial.println("Benchmark send");
}

void pc_wifi_interface_send_img(image_t* img, const char* title) {
    // Wait until pc is ready
    log_d("Device wants to send image title");
    log_d("pc_is_ready byte: %d", pc_is_ready);
    while(!(pc_is_ready & PC_IS_READY_FOR_IMG_TITLE) && wifi_client.connected()) {
        pc_wifi_interface_process_rx_complete();
        pc_wifi_interface_rx();
    }

    // stop when pc has disconnected
    if(!wifi_client.connected())
        return;

    // Clear flags
    pc_is_ready = 0;

    pc_wifi_interface_write(IMAGE_TITLE_ACK, (uint8_t*)title, strlen(title));
    log_d("image title send");

    // Wait until PC is ready
    log_d("Device wants to send image struct");
    log_d("pc_is_ready byte: %d", pc_is_ready);
    while(!(pc_is_ready & PC_IS_READY_FOR_IMG_STRUCT) && wifi_client.connected()) {
        pc_wifi_interface_process_rx_complete();
        pc_wifi_interface_rx();
    }

    // stop when pc has disconnected
    if(!wifi_client.connected())
        return;

    // Clear flags
    pc_is_ready = 0;

    pc_wifi_interface_write(IMAGE_STRUCT_ACK, (uint8_t*)img, sizeof(image_t));
    log_d("image struct send");

    // Wait until pc is ready
    log_d("Device wants to send image data");
    log_d("pc_is_ready byte: %d", pc_is_ready);
    while(!(pc_is_ready & PC_IS_READY_FOR_IMG_DATA) && wifi_client.connected()) {
        pc_wifi_interface_process_rx_complete();
        pc_wifi_interface_rx();
    }

    // stop when pc has disconnected
    if(!wifi_client.connected())
        return;

    // Clear flags
    pc_is_ready = 0;

    uint32_t len = img->cols * img->rows;

    if(img->type == IMGTYPE_RGB565) {
        len = sizeof(rgb565_pixel_t) * img->cols * img->rows;
    } else if(img->type == IMGTYPE_RGB888) {
        len = sizeof(rgb888_pixel_t) * img->cols * img->rows;
    }

    pc_wifi_interface_write(IMAGE_DATA_ACK, img->data, len);
    log_d("image data send");
}