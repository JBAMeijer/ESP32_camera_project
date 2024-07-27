#ifndef BENCHMARK_H_
#define BENCHMARK_H_

#include "general.h"

#define BENCHMARK_MAX_STRING_LENGTH 32

// 40 bytes - no padding
typedef struct {
    u64 start;
    u64 stop;
    s8 name[BENCHMARK_MAX_STRING_LENGTH];
} benchmark_t;

benchmark_t create_benchmark_from_data(const s8 *data);
void benchmark_start(benchmark_t *b, const s8 *name);
void benchmark_stop(benchmark_t *b);
size_t benchmark_tostr(benchmark_t *b, s8 *str);
size_t benchmark_tostr_ms(benchmark_t *b, s8 *str);

#endif /* BENCHMARK_H_ */
