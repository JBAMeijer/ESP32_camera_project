#include "benchmark.h"
#include <raylib.h>

#include <string.h>
#include <stdlib.h>

benchmark_t create_benchmark_from_data(const u8* data) {
    benchmark_t temp;
    uint32_t start_time = 0, stop_time = 0;
    start_time |= (u32)data[0];
    start_time |= (u32)data[1] << 8;
    start_time |= (u32)data[2] << 16;
    start_time |= (u32)data[3] << 24;

    stop_time |= (u32)data[4];
    stop_time |= (u32)data[5] << 8;
    stop_time |= (u32)data[6] << 16;
    stop_time |= (u32)data[7] << 24;

    temp.start = start_time;
    temp.stop = stop_time;
    strncpy(temp.name, &data[8], (BENCHMARK_MAX_STRING_LENGTH));

    return temp;
}

void benchmark_start(benchmark_t* b, const u8* name) {
    strncpy((s8*)b->name, name, (BENCHMARK_MAX_STRING_LENGTH-1));
    b->name[(BENCHMARK_MAX_STRING_LENGTH-1)]='\0';
    b->start = GetTime() * 1000000;
}

void benchmark_stop(benchmark_t* b) {
    b->stop = GetTime() * 1000000;
}

size_t benchmark_tostr(benchmark_t* b, u8* str) {
    u8 tmp[16];

    strcpy(str, "Benchmark: ");
    strcat(str, b->name);
    strcat(str, " ");
    sprintf(tmp, "%d", b->stop - b->start);
    strcat(str, tmp);
    strcat(str, " us\r\n");

    return strlen(str);
}

size_t benchmark_tostr_ms(benchmark_t* b, u8* str) {
    u8 tmp[16];

    strcpy(str, "Benchmark: ");
    strcat(str, b->name);
    strcat(str, " ");
    sprintf(tmp, "%d", (b->stop - b->start) / 1000);
    strcat(str, tmp);
    strcat(str, " ms\r\n");

    return strlen(str);
}