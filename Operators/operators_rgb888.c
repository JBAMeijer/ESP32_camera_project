#if !defined(ESP32_S3)
#include "operators_rgb888.h"
#include <stdlib.h> 
#else
    #include <Arduino.h>
    #include <esp_camera.h>
    #include "operators/operators_rgb565.h"
#endif

#if defined(USE_MEM_MANAGER)
	#include "mem_manager.h"
#endif

image_t *newRGB888Image(const uint32_t cols, const uint32_t rows) {
    image_t *img = (image_t*)malloc(sizeof(image_t));
    if(img == NULL) {
        return NULL;
    }

#if defined(USE_MEM_MANAGER)
    img->data = mem_manager_alloc();
#else
    img->data = (uint8_t*)malloc((rows * cols) * sizeof(rgb888_pixel_t));
#endif

    if(img->data == NULL) {
        free(img);
        return NULL;
    }

    img->cols = cols;
    img->rows = rows;
    img->view = IMGVIEW_CLIP;
    img->type = IMGTYPE_RGB888;
    return(img);
}
