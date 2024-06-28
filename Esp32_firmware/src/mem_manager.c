#ifdef __cplusplus
    extern "C" {
#endif


#include "mem_manager.h"
#include "Arduino.h"
#include <stdio.h>

#define BLOCK_SIZE (320 * 240 * 3)

#define SDRAM_SIZE (4 * 1024 * 1024) // 4MB

#define N_DATA_BLOCKS (SDRAM_SIZE / BLOCK_SIZE)

typedef enum {
    FREE = 0,
    TAKEN = 1,
}eBlockStatus;

//__attribute__((section(".img_data")))
//uint8_t data_blocks[1][BLOCK_SIZE];
uint8_t* data_blocks[N_DATA_BLOCKS];

eBlockStatus block_status[N_DATA_BLOCKS];

void mem_manager_init(void) {
    for(int i=0; i < N_DATA_BLOCKS; i++) {
        data_blocks[i] = (uint8_t*)ps_malloc(BLOCK_SIZE);
        block_status[i] = FREE;
    }
}

uint8_t *mem_manager_alloc(void) {
    uint8_t *ret = NULL;

    int i = 0;
    while((block_status[i] == TAKEN) && (i < N_DATA_BLOCKS)) {
        i++;
    }

    if(i < N_DATA_BLOCKS) {
        block_status[i] = TAKEN;
        ret = data_blocks[i];
    }

    return ret;
}

void mem_manager_free(uint8_t *data) {
    // Search for the address of the block
    int i=0;
    while((data_blocks[i] != data) && (i < N_DATA_BLOCKS)) {
        i++;
    }
    
    // Mark the block as free
    if(i < N_DATA_BLOCKS) {
        block_status[i] = FREE;
    }
}

uint8_t mem_manager_free_blocks(void) {
    int n=0;
    
    // Count the number of free data blocks
    for(int i=0; i < N_DATA_BLOCKS; i++) {
        if(block_status[i] == FREE) {
            n++;
        }
    }
    
    return n;
}

#ifdef __cplusplus
}
#endif
