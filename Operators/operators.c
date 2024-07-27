#ifdef __cplusplus
    extern "C" {
#endif

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#if !defined(ESP32_S3)
#include "operators.h"
#include "operators_basic.h"
#include "operators_rgb888.h"
#include "operators_rgb565.h"
#else
#include "operators/operators.h"
#endif

void delete_image(image_t *img) {
    switch(img->type) 
    {
        case IMGTYPE_BASIC:
            delete_basic_image(img);
            break;
        case IMGTYPE_INT16:
        case IMGTYPE_FLOAT:
            assert(false && "delete_image(): no corresponding delete function for INT16 or FLOAT!");
            break;
        case IMGTYPE_RGB565:
            delete_rgb565_image(img);
            break;
        case IMGTYPE_RGB888:
            delete_rgb888_image(img);
            break;
    }
}

void convert_image(const image_t *src, image_t *dst) {
    switch(dst->type)
    {
        case IMGTYPE_BASIC:
            convert_to_basic_image(src, dst);
        break;
        case IMGTYPE_INT16:
        case IMGTYPE_FLOAT:
        case IMGTYPE_RGB565:
        case IMGTYPE_RGB888:
            assert(false && "convert_image(): image type FLOAT, INT16, RGB565 and RGB888 not supported!");
            break;
        default: 
            assert(false && "convert_image(): unknown image type!");
            break;
    }
}

void copy(const image_t *src, image_t *dst) {
#if !defined(USE_MEM_MANAGER)
    assert(src->type == dst->type && "copy(): src and dst are of different types!");
#endif
    switch(src->type)
    {
        case IMGTYPE_BASIC:
            copy_basic(src, dst);
            break;
        case IMGTYPE_INT16:
        case IMGTYPE_FLOAT:
            assert(false && "copy(): image type FLOAT and INT16 not supported!");
            break;
        case IMGTYPE_RGB565:
            assert(false && "copy(): image type rgb565 not ready yet!");
            //copy_rgb565(src, dst);
            break;
        case IMGTYPE_RGB888:
            //assert(false && "copy(): image type rgb888 not ready yet!");
            copy_rgb888(src, dst);
            break;
        default: 
            assert(false && "copy(): unknown image type!");
            break;
    }
}

void threshold(const image_t *src, image_t *dst, u8 threshold_value) {
    assert(src->type == dst->type && "threshold(): src and dst are of different types!");
    
    switch(src->type) 
    {
        case IMGTYPE_BASIC:
            threshold_basic(src, dst, (basic_pixel_t)threshold_value);
            break;
        case IMGTYPE_INT16:
        case IMGTYPE_FLOAT:
        case IMGTYPE_RGB565:
        case IMGTYPE_RGB888:
            assert(false && "threshold(): image type FLOAT, INT16, RGB565 and RGB888 not supported!");
            break;
        default: 
            assert(false && "threshold(): unknown image type!");
            break;
    }

}

#ifdef __cplusplus
}
#endif
