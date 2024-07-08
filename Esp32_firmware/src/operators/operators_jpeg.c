#if defined(BUILD_FROM_GUI)
#include "operators.h"
#include <stdlib.h> 
#else
    #include <Arduino.h>
    #include <esp_camera.h>
    #include "operators/operators_rgb565.h"


    #if defined(USE_MEM_MANAGER)
    #include "mem_manager.h"
    #endif
#endif

image_t *newJPEGImageFromData(const uint8_t* data, const uint32_t cols, const uint32_t rows, const uint32_t len) {
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
    img->len  = len;
    img->view = IMGVIEW_CLIP;
    img->type = IMGTYPE_JPEG;

    //register long int i = cols * rows;
    register int32_t i = len;
    register basic_pixel_t *s = (basic_pixel_t *)data;
    register basic_pixel_t *d = (basic_pixel_t *)img->data;

    while(i-- > 0)
        *d++ = *s++;

    return(img);
}

void deleteJPEGImage(image_t* img) {
#if defined(USE_MEM_MANAGER)
    mem_manager_free(img->data);
#else
    free(img->data);
#endif

    free(img);
}