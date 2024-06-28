#include <Arduino.h>
#include <esp_camera.h>
#include "operators_rgb565.h"


#if defined(USE_MEM_MANAGER)
#include "mem_manager.h"
#endif

image_t *newRGB565Image(const uint32_t cols, const uint32_t rows) {
    image_t *img = (image_t*)malloc(sizeof(image_t));
    if(img == NULL) {
        return NULL;
    }

#if defined(USE_MEM_MANAGER)
    img->data = mem_manager_alloc();
#else
    img->data = (uint8_t*)malloc((rows * cols) * sizeof(rgb565_pixel_t));
#endif

    if(img->data == NULL) {
        free(img);
        return NULL;
    }

    img->cols = cols;
    img->rows = rows;
    img->view = IMGVIEW_CLIP;
    img->type = IMGTYPE_RGB565;
    return(img);
}

void copy_framebuffer_to_rgb656Image(const camera_fb_t* bf, image_t* image) {
    register long int i = bf->width * bf->height;
    register rgb565_pixel_t *s = (rgb565_pixel_t *)bf->buf;
    register rgb565_pixel_t *d = (rgb565_pixel_t *)image->data;

    image->rows;
    image->cols;
    image->type = IMGTYPE_RGB565;
    image->view = IMGVIEW_CLIP;

    while(i-- > 0)
        *d++ = *s++;
}


