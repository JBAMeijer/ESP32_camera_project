#ifndef MEM_MANAGER_H_
#define MEM_MANAGER_H_

#include <stdint.h>
#include "general.h"
#include "operators.h"

#if !defined(N_REQUIRED_DATA_BLOCKS)
#error Define the amount of data blocks inside of the build.zig with [N_REQUIRED_DATA_BLOCKS]
#endif

#define BLOCK_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT * 3)

int mem_manager_init(void);
image_t *mem_manager_alloc(void);
void mem_manager_free(image_t *data);
u8 mem_manager_free_blocks(void);

#endif // MEM_MANAGER_H_
