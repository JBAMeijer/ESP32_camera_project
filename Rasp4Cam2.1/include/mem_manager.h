#ifndef MEM_MANAGER_H_
#define MEM_MANAGER_H_

#include <stdint.h>

int mem_manager_init(void);
uint8_t* mem_manager_alloc(void);
void mem_manager_free(uint8_t* data);
uint8_t mem_manager_free_blocks(void);

#endif // MEM_MANAGER_H_
