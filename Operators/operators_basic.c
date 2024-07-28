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

void contrast_stretch_fast_basic(const image_t *src, image_t *dst) {
	basic_pixel_t LUT[256];
	register basic_pixel_t lpixel = 255, hpixel = 0;
	register u32 *pixelsrc = (u32 *)src->data;
	
	s32 i = (dst->cols * dst->rows) / 4;
	while(i-- > 0) {
		if(((*pixelsrc & 0xff000000UL) >> 24) < lpixel) lpixel = ((*pixelsrc & 0xff000000UL) >> 24);
		if(((*pixelsrc & 0xff000000UL) >> 24) > hpixel) hpixel = ((*pixelsrc & 0xff000000UL) >> 24);
		
		if(((*pixelsrc & 0x00ff0000UL) >> 16) < lpixel) lpixel = ((*pixelsrc & 0x00ff0000UL) >> 16);
		if(((*pixelsrc & 0x00ff0000UL) >> 16) > hpixel) hpixel = ((*pixelsrc & 0x00ff0000UL) >> 16);
		
		if(((*pixelsrc & 0x0000ff00UL) >> 8) < lpixel) lpixel = ((*pixelsrc & 0x0000ff00UL) >> 8);
		if(((*pixelsrc & 0x0000ff00UL) >> 8) > hpixel) hpixel = ((*pixelsrc & 0x0000ff00UL) >> 8);
		
		if(((*pixelsrc & 0x000000ffUL)) < lpixel) lpixel = ((*pixelsrc & 0x000000ffUL));
		if(((*pixelsrc & 0x000000ffUL)) > hpixel) hpixel = ((*pixelsrc & 0x000000ffUL));
		++pixelsrc;
	}
	
	pixelsrc = (u32 *)src->data;
	register u32 *pixeldst = (u32 *)dst->data;
	i = (dst->cols * dst->rows) / 4;
	if(lpixel != hpixel) {
		register const float stretchfactor = 255 / (float)(hpixel - lpixel);
		
		for(u32 t = 0; t <= hpixel - lpixel; t++)
			LUT[t + lpixel] = (basic_pixel_t)((t * stretchfactor) + 0.5f);
			
		while(i-- > 0) {
			*pixeldst = (LUT[((*pixelsrc & 0xff000000UL) >> 24)] << 24) |
						(LUT[((*pixelsrc & 0x00ff0000UL) >> 16)] << 16) |
						(LUT[((*pixelsrc & 0x0000ff00UL) >>  8)]  << 8) |
						(LUT[((*pixelsrc & 0x000000ffUL)      )]      );
			++pixelsrc;
			++pixeldst;
		}
	} else {
		while(i-- > 0)
			*pixeldst = 128;
		pixeldst++;
	}
}
