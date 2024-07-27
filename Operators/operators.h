#ifdef __cplusplus
    extern "C" {
#endif

#ifndef _OPERATORS_H_
#define _OPERATORS_H_

#include "stdint.h"
#include "general.h"

#if defined(RASP4)
    #define USE_MEM_MANAGER
#endif

#if defined(ESP32_S3)
    #include <esp_camera.h>
    #include "config.h"

    #if defined(CAMERA_MODEL_XIAO_ESP32S3)
        #define USE_MEM_MANAGER
    #endif
#endif

// ----------------------------------------------------------------------------
// Type definitions
// ----------------------------------------------------------------------------
typedef enum
{
    IMGTYPE_BASIC  = 0,  // Unsigned Character
    IMGTYPE_INT16  = 1,  // Integer
    IMGTYPE_FLOAT  = 2,  // Float
    IMGTYPE_RGB888 = 3,  // RGB 24 bpp
    IMGTYPE_RGB565 = 4,  // RGB 16 bpp
  
    IMGTYPE_MAX    = 2147483647 // Max 32-bit int value,
                                // forces enum to be 4 bytes
}eImageType;

static const s8* const IMGTYPE_NAMES[] = {
    "IMGTYPE_BASIC",
    "IMGTYPE_INT16",
    "IMGTYPE_FLOAT",
    "IMGTYPE_RGB888",
    "IMGTYPE_RGB565",
};

typedef enum
{
    IMGVIEW_STRETCH = 0,
    IMGVIEW_CLIP    = 1,
    IMGVIEW_BINARY  = 2,
    IMGVIEW_LABELED = 3,
  
    IMGVIEW_MAX     = 2147483647 // Max 32-bit int value,
                                 // forces enum to be 4 bytes
}eImageView;

static const s8* const IMGVIEW_NAMES[] = {
    "IMGVIEW_STRETCH",
    "IMGVIEW_CLIP",
    "IMGVIEW_BINARY",
    "IMGVIEW_LABELED",
};

// Pixel types
typedef u8  basic_pixel_t;
typedef s16 int16_pixel_t;
typedef f32 float_pixel_t;

typedef struct rgb888_pixel_t
{
    u8 r;
    u8 g;
    u8 b;
    
}rgb888_pixel_t;

typedef u16 rgb565_pixel_t;

typedef struct complex_pixel_t
{
    f32 real;
    f32 imaginary;
    
}complex_pixel_t;

// Image type
typedef struct
{
    s32         cols;
    s32         rows;
    s32         len;
    eImageView  view;
    eImageType  type;
    u8          *data;
    
}image_t;

// Creates a zero initialized image with the specified cols and rows
image_t *newBasicImage(const u32 cols, const u32 rows);
image_t *newRGB565Image(const u32 cols, const u32 rows);
image_t *newRGB888Image(const u32 cols, const u32 rows); 

void delete_image(image_t *img);

void convert_image(const image_t *src, image_t *dst);

void copy(const image_t *src, image_t *dst);

void threshold(const image_t *src, image_t *dst, u8 threshold_value);

#endif // _OPERATORS_H_

#ifdef __cplusplus
 }
#endif
