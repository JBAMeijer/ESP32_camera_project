#include "mem_manager.h"
#include <sys/resource.h>
#include <stdio.h>

#define BLOCK_SIZE (640 * 480 * 3)

#define STACK_SIZE (8 * 1024 * 1024) // 8MB

#define N_DATA_BLOCKS (STACK_SIZE / BLOCK_SIZE)

typedef enum {
	FREE = 0,
	TAKEN = 1,
}eBlockStatus;

uint8_t data_blocks[N_DATA_BLOCKS][BLOCK_SIZE];

eBlockStatus block_status[N_DATA_BLOCKS];

int mem_manager_init(void) {
	printf("BLOCK_SIZE: %d\n", BLOCK_SIZE);
	printf("MEMORY_SIZE: %d\n", STACK_SIZE);
	printf("N_DATA_BLOCKS: %d\n", N_DATA_BLOCKS);
	struct rlimit rl;
    int result;
    
    if(getrlimit(RLIMIT_STACK, &rl) == 0) {
		printf("Stack size: %d\n", rl.rlim_cur);
		if(rl.rlim_cur < STACK_SIZE * 2) {
			rl.rlim_cur = STACK_SIZE * 2;
			if(setrlimit(RLIMIT_STACK, &rl) != 0) {
				printf("Failed to set the required stack size\n");
				return (-1);
			} else {
				printf("Stack size set to: %d\n", rl.rlim_cur);
			}
		}
    }
	
	for(int i = 0; i < N_DATA_BLOCKS; i++) {
		block_status[i] = FREE;
	}
	
	return (0);
}

uint8_t* mem_manager_alloc(void) {
	uint8_t* ret = NULL;
	
	int i=0;
	while((block_status[i] == TAKEN) && (i < N_DATA_BLOCKS)) i++;
		
	if(i < N_DATA_BLOCKS) {
		block_status[i] = TAKEN;
		ret = data_blocks[i];
		printf("Memory block %d status set to TAKEN\n", i);
	}
	
	return ret;
}

void mem_manager_free(uint8_t* data) {
	int i = 0;
	while((data_blocks[i] != data) && (i < N_DATA_BLOCKS)) i++;
	
	if(i < N_DATA_BLOCKS) block_status[i] = FREE;
}

uint8_t mem_manager_free_blocks(void) {
	int n=0;
	
	for(int i = 0; i < N_DATA_BLOCKS; i++)
	{
		if(block_status[i] == FREE) n++;
	}
	
	return n;
}
