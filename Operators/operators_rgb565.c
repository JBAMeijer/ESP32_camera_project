#if !defined(ESP32_S3)
    #include "operators_rgb565.h"
    #include <stdlib.h>
#else
    #include <Arduino.h>
    #include <esp_camera.h>
    #include "operators/operators_rgb565.h"
#endif

#if defined(USE_MEM_MANAGER)
    #include "mem_manager.h"
#endif

image_t *newRGB565Image(const u32 cols, const u32 rows) {
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
    img->data = (u8*)malloc((rows * cols) * sizeof(rgb565_pixel_t));
    if(img->data == NULL) {
        free(img);
        return NULL;
    }
#endif

    img->cols = cols;
    img->rows = rows;
    img->view = IMGVIEW_CLIP;
    img->type = IMGTYPE_RGB565;
    return(img);
}

void delete_rgb565_image(image_t *img) {
#if defined(USE_MEM_MANAGER)
	mem_manager_free(img);
#else
	free(img->data);
	free(img);
#endif

}
