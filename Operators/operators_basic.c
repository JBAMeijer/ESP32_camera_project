#if !defined(ESP32_S3)
	#include "operators_basic.h"
	#include <stdlib.h>
#else
    #include <Arduino.h>
    #include <esp_camera.h>
    #include "operators/operators_basic.h"
#endif

#if defined(USE_MEM_MANAGER)
	#include "mem_manager.h"
#endif

#include <stdio.h>

image_t *newBasicImage(const u32 cols, const u32 rows) {
	image_t *img = NULL;
	
#if defined(USE_MEM_MANAGER)
    img = mem_manager_alloc();
    if(img == NULL) {
		return (NULL);
	}
#else
	img = (image_t*)malloc(sizeof(image_t));
	if(img == NULL) {
		return (NULL);
	}
    img->data = (u8*)malloc((rows * cols) * sizeof(basic_pixel_t));
    
    if(img->data == NULL) {
        free(img);
        return NULL;
    }
#endif

    img->cols = cols;
    img->rows = rows;
    img->view = IMGVIEW_CLIP;
    img->type = IMGTYPE_BASIC;
    return(img);
}

void delete_basic_image(image_t *img) {
#if defined(USE_MEM_MANAGER)
	mem_manager_free(img);
#else
	free(img->data);
	free(img);
#endif

}

void convert_to_basic_image(const image_t *src, image_t *dst) {
	register s32 i = src->rows * src->cols;
	register basic_pixel_t *d = (basic_pixel_t*)dst->data;
	
	dst->view = src->view;
	dst->cols = src->cols;
	dst->rows = src->rows;
	dst->view = src->view;
	
	switch(src->type)
	{
	case IMGTYPE_RGB888:
	{
		rgb888_pixel_t *s = (rgb888_pixel_t*)src->data;
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

void copy_basic(const image_t *src, image_t *dst) {
	register s32 i = src->rows * src->cols / 4;
	register u32 *pixelsrc = (u32*)src->data;
	register u32 *pixeldst = (u32*)dst->data;
	
	dst->rows = src->rows;
	dst->cols = src->cols;
	dst->type = src->type;
	dst->view = src->view;
	
	while(i-- > 0)
		*pixeldst++ = *pixelsrc++;
}

void threshold_basic(const image_t *src, image_t *dst, u8 threshold_value) {
	register s32 i = src->rows * src->cols / 4;
	register u32 *pixelsrc = (u32*)src->data;
	register u32 *pixeldst = (u32*)dst->data;
	
	while(i-- > 0) {
		*pixeldst  = (((*pixelsrc & 0xFF000000) >> 24) >= threshold_value) * 0x01000000;
		*pixeldst |= (((*pixelsrc & 0xFF0000)   >> 16) >= threshold_value) * 0x010000;
		*pixeldst |= (((*pixelsrc & 0xFF00) 	>> 8)  >= threshold_value) * 0x0100;
		*pixeldst |= (((*pixelsrc & 0xFF)) 			   >= threshold_value) * 0x01;
		
		pixeldst++;
		pixelsrc++;
	}
	
	dst->view = IMGVIEW_BINARY;
	
}
