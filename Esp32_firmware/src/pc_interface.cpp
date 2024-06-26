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
        register uint32_t cnt = len;
        register uint32_t n;
        register uint32_t index = 0;

        while((cnt > 0) && (wifi_client.connected())) {
            if(cnt > 256) n = 256;
            else n = cnt;

            bytes_send = wifi_client.write(&buffer[index], n);
            if(bytes_send != n)
                return;
            
            index += n;
            cnt -= n;
        }
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
        case BENCHMARK_REQ:
        {
            // Set flag
            Serial.println("Benchmark flag set");
            pc_is_ready |= PC_IS_READY_FOR_BENCHMARK;
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