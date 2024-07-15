#if !defined(ESP32_S3)
#include "operators_basic.h"
#include <stdlib.h>
#else
    #include <Arduino.h>
    #include <esp_camera.h>
    #include "operators/operators_rgb565.h"
#endif

#if defined(USE_MEM_MANAGER)
	#include "mem_manager.h"
#endif

#include <stdio.h>

image_t* newBasicImage(const uint32_t cols, const uint32_t rows) {
	image_t* img = (image_t*)malloc(sizeof(image_t));
	if(img == NULL) {
		return (NULL);
	}
	
#if defined(USE_MEM_MANAGER)
    img->data = mem_manager_alloc();
#else
    img->data = (uint8_t*)malloc((rows * cols) * sizeof(basic_pixel_t));
#endif

	if(img->data == NULL) {
        free(img);
        return NULL;
    }

    img->cols = cols;
    img->rows = rows;
    img->view = IMGVIEW_CLIP;
    img->type = IMGTYPE_BASIC;
    return(img);
}

void convert_to_basic_image(const image_t* src, image_t* dst) {
	register long int i = src->rows * src->cols;
	register basic_pixel_t* d = (basic_pixel_t*)dst->data;
	
	dst->view = src->view;
	dst->cols = src->cols;
	dst->rows = src->rows;
	dst->view = src->view;
	
	switch(src->type)
	{
	case IMGTYPE_RGB888:
	{
		rgb888_pixel_t* s = (rgb888_pixel_t*)src->data;
		while(i-- > 0) {
			unsigned char r = s->r;
			unsigned char g = s->g;
			unsigned char b = s->b;
			
			*d++ = (basic_pixel_t)(0.3f * r + 0.59f * g + 0.11f * b);
			s++;
		}
	}break;
	default:
		printf("Unsupported image type to basic");
		break;
	}

}

