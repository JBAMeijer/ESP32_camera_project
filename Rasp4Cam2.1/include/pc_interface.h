#ifndef PC_INTERFACE_H_
#define PC_INTERFACE_H_

#include <stdbool.h>
#include "benchmark.h"

typedef enum
{
    KEEP_ALIVE_REQ = 0,
    KEEP_ALIVE_ACK,
    CONNECT_REQ,
    CONNECT_ACK,
    DISCONNECT_CMD,
    VERSION_REQ,
    VERSION_ACK,
    SET_DEVICE_MODE_REQ,
    SET_DEVICE_MODE_ACK,
    IMAGE_TITLE_REQ,
    IMAGE_TITLE_ACK,
    IMAGE_STRUCT_REQ,
    IMAGE_STRUCT_ACK,
    IMAGE_DATA_REQ,
    IMAGE_DATA_ACK,
    BENCHMARK_REQ,
    BENCHMARK_ACK,
    IMAGE_TITLE_OR_BENCHMARK_REQ,
}eFrameType;

typedef enum
{
    MODE_DISCONNECTED = 0,
    MODE_CONNECTED,
    MODE_SNAPSHOT,
    MODE_STOP,
}eDeviceMode;

s32 pc_wifi_interface_start();
void pc_wifi_interface_stop();
void pc_wifi_interface_reset();
void pc_wifi_interface_update();
void pc_wifi_interface_send_benchmark(benchmark_t *benchmark);
//void pc_wifi_interface_send_img(image_t* img, const char* title);

#endif /* PC_INTERFACE_H_ */
