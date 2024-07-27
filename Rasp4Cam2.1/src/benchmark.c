#include "benchmark.h"
//#include <raylib.h>

#include <string.h>
#include <stdlib.h>
#include <time.h>

benchmark_t create_benchmark_from_data(const s8 *data) {
    benchmark_t temp;
    u64 start_time = 0, stop_time = 0;
    start_time |= (u64)data[0];
    start_time |= (u64)data[1] << 8;
    start_time |= (u64)data[2] << 16;
    start_time |= (u64)data[3] << 24;
    start_time |= (u64)data[4] << 32;
    start_time |= (u64)data[5] << 40;
    start_time |= (u64)data[6] << 48;
    start_time |= (u64)data[7] << 56;

    stop_time |= (u64)data[8];
    stop_time |= (u64)data[9]  << 8;
    stop_time |= (u64)data[10] << 16;
    stop_time |= (u64)data[11] << 24;
    stop_time |= (u64)data[12] << 32;
    stop_time |= (u64)data[13] << 40;
    stop_time |= (u64)data[14] << 48;
    stop_time |= (u64)data[15] << 56;

    temp.start = start_time;
    temp.stop = stop_time;
    strncpy(temp.name, &data[16], (BENCHMARK_MAX_STRING_LENGTH));

    return temp;
}

void benchmark_start(benchmark_t *b, const s8 *name) {
    strncpy((s8*)b->name, name, (BENCHMARK_MAX_STRING_LENGTH-1));
    b->name[(BENCHMARK_MAX_STRING_LENGTH-1)]='\0';
    clock_t now = clock();
    
    //double time = GetTime();
    b->start = now;
}

void benchmark_stop(benchmark_t *b) {
    clock_t now = clock();
    //double time = GetTime();
    b->stop = now;
}

size_t benchmark_tostr(benchmark_t *b, s8 *str) {
    s8 tmp[16];

    strcpy(str, "Benchmark: "); 
    strcat(str, b->name);
    strcat(str, " ");
    sprintf(tmp, "%llu", (u64)((double)(b->stop - b->start)/CLOCKS_PER_SEC*1000000));
    strcat(str, tmp);
    strcat(str, " us");

    return strlen(str);
}

size_t benchmark_tostr_ms(benchmark_t *b, s8 *str) {
    s8 tmp[16];

    strcpy(str, "Benchmark: ");
    strcat(str, b->name);
    strcat(str, " ");
    sprintf(tmp, "%llu", (u64)((double)(b->stop - b->start)/CLOCKS_PER_SEC*1000));
    strcat(str, tmp);
    strcat(str, " ms");

    return strlen(str);
}
