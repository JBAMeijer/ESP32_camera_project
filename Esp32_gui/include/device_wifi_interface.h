#ifndef DEVICE_INTERFACE_H_
#define DEVICE_INTERFACE_H_

#include <stdbool.h>

#include "general.h"
#include "benchmark.h"


typedef enum {
    IDLE,
    REQUESTED,
    ACKNOWLEDGED,
} eDeviceDataState;

typedef enum {
    WAIT_FOR_HEADER,
    WAIT_FOR_DATA,
} rx_state_t;

typedef enum {
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
} eFrameType;

typedef void (*benchmarkFuncDef)(benchmark_t);
typedef void (*deviceVersionFuncDef)(s32, s32, s32);

s32 open_connection(deviceVersionFuncDef, benchmarkFuncDef);
void close_connection(void);
void req_device_version(void);
void pc_wifi_interface_rx(void);
void pc_wifi_interface_process_rx_complete(void);
void send_keepalive(void);
void send_req_for_new_benchmark(void);
void handle_received_benchmark_data(void);

void device_wifi_interface_update(void);

bool connected(void);

#endif