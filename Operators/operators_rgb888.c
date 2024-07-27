#if !defined(ESP32_S3)
	#include "operators_rgb888.h"
	#include <stdlib.h> 
#else
    #include <Arduino.h>
    #include <esp_camera.h>
    #include "operators/operators_rgb888.h"
#endif

#if defined(USE_MEM_MANAGER)
	#include "mem_manager.h"
#endif

image_t *newRGB888Image(const u32 cols, const u32 rows) {
    image_t *img = NULL;
    
#if defined(USE_MEM_MANAGER)
    img = mem_manager_alloc();
    if(img == NULL) {
        return NULL;
    }
#else
	img = (image_t*)malloc(sizeof(image_t));
    if(img == NULL) {
        return NULL;
    }
    img->data = (u8*)malloc((rows * cols) * sizeof(rgb888_pixel_t));
    if(img->data == NULL) {
        free(img);
        return NULL;
    }
#endif


    img->cols = cols;
    img->rows = rows;
    img->view = IMGVIEW_CLIP;
    img->type = IMGTYPE_RGB888;
    return(img);
}

void delete_rgb888_image(image_t *img) {
#if defined(USE_MEM_MANAGER)
	mem_manager_free(img);
#else
	free(img->data);
	free(img);
#endif

}

void copy_rgb888(const image_t *src, image_t *dst) {
	register s32 i = src->rows * src->cols;
	register rgb888_pixel_t *pixelsrc = (rgb888_pixel_t*)src->data;
	register rgb888_pixel_t *pixeldst = (rgb888_pixel_t*)dst->data;
	
	dst->rows = src->rows;
	dst->cols = src->cols;
	dst->type = src->type;
	dst->view = src->view;
	
	while(i-- > 0)
		*pixeldst++ = *pixelsrc++;
}
