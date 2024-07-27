#include "mem_manager.h"
#include "vision.h"
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
	FREE = 0,
	TAKEN = 1,
}eBlockStatus;

image_t images[N_REQUIRED_DATA_BLOCKS];
u8 *data_blocks[N_REQUIRED_DATA_BLOCKS];
eBlockStatus block_status[N_REQUIRED_DATA_BLOCKS];

int mem_manager_init(void) {
	printf("BLOCK_SIZE: %d\n", BLOCK_SIZE);
	printf("N_DATA_BLOCKS: %d\n", N_REQUIRED_DATA_BLOCKS);
	printf("TOTAL_MEMORY_SIZE: %d\n", BLOCK_SIZE*N_REQUIRED_DATA_BLOCKS);
	
	for(s32 i = 0; i < N_REQUIRED_DATA_BLOCKS; i++) {
		data_blocks[i] = (u8*)malloc(BLOCK_SIZE);
		images[i].data = data_blocks[i];
		block_status[i] = FREE;
	}
	
	return (0);
}

image_t *mem_manager_alloc(void) {
	image_t *ret = NULL;
	
	s32 i=0;
	while((block_status[i] == TAKEN) && (i < N_REQUIRED_DATA_BLOCKS)) i++;
		
	if(i < N_REQUIRED_DATA_BLOCKS) {
		block_status[i] = TAKEN;
		ret = &images[i];
		//ret = data_blocks[i];
		printf("Memory block %d status set to TAKEN\n", i);
	}
	
	return ret;
}

void mem_manager_free(image_t *data) {
	s32 i = 0;
	while((&images[i] != data) && (i < N_REQUIRED_DATA_BLOCKS)) i++;
	
	if(i < N_REQUIRED_DATA_BLOCKS) block_status[i] = FREE;
}

u8 mem_manager_free_blocks(void) {
	u8 n=0;
	
	for(s32 i = 0; i < N_REQUIRED_DATA_BLOCKS; i++)
	{
		if(block_status[i] == FREE) n++;
	}
	
	return n;
}
