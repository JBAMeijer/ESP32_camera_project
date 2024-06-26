#include "benchmark.h"

void benchmark_start(benchmark_t* b, const char* name) {
    strncpy(b->name, name, (BENCHMARK_MAX_STRING_LENGTH-1));
    b->name[(BENCHMARK_MAX_STRING_LENGTH-1)]='\0';
    b->start = micros();
}

void benchmark_stop(benchmark_t* b) {
    b->stop = micros();
}

size_t benchmark_tostr(benchmark_t* b, char* str) {
    char tmp[16];

    strcpy(str, "benchmark: ");
    strcat(str, b->name);
    strcat(str, " ");
    itoa(b->stop - b->start, tmp, 10);
    strcat(str, tmp);
    strcat(str, " us\r\n");

    return strlen(str);
}