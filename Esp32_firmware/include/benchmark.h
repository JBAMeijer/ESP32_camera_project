#ifndef BENCHMARK_H_
#define BENCHMARK_H_

#include <Arduino.h>

#define BENCHMARK_MAX_STRING_LENGTH 32

typedef struct {
    uint32_t start;
    uint32_t stop;
    char name[BENCHMARK_MAX_STRING_LENGTH];
} benchmark_t;

void benchmark_start(benchmark_t* b, const char* name);
void benchmark_stop(benchmark_t* b);
size_t benchmark_tostr(benchmark_t* b, char* str);

#endif /* BENCHMARK_H_ */