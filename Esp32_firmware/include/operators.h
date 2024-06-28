#ifdef __cplusplus
    extern "C" {
#endif

#ifndef _OPERATORS_H_
#define _OPERATORS_H_

#include "stdint.h"
#include "config.h"
#include <esp_camera.h>

#if defined(CAMERA_MODEL_XIAO_ESP32S3)
#define USE_MEM_MANAGER
#endif

// ----------------------------------------------------------------------------
// Type definitions
// ----------------------------------------------------------------------------
typedef enum
{
    IMGTYPE_BASIC  = 0,  // Unsigned Character
    IMGTYPE_INT16  = 1,  // Integer
    IMGTYPE_FLOAT  = 2,  // Float
    IMGTYPE_RGB888 = 3,  // RGB 8-bit per pixel
    IMGTYPE_RGB565 = 4,
  
    IMGTYPE_MAX    = 2147483647 // Max 32-bit int value,
                                // forces enum to be 4 bytes
}eImageType;

typedef enum
{
    IMGVIEW_STRETCH = 0,
    IMGVIEW_CLIP    = 1,
    IMGVIEW_BINARY  = 2,
    IMGVIEW_LABELED = 3,
  
    IMGVIEW_MAX     = 2147483647 // Max 32-bit int value,
                                 // forces enum to be 4 bytes
}eImageView;

// Pixel types
typedef uint8_t basic_pixel_t;
typedef int16_t int16_pixel_t;
typedef float float_pixel_t;

typedef struct rgb888_pixel_t
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    
}rgb888_pixel_t;

typedef uint16_t rgb565_pixel_t;

typedef struct complex_pixel_t
{
    float real;
    float imaginary;
    
}complex_pixel_t;

// Image type
typedef struct
{
    int32_t     cols;
    int32_t     rows;
    eImageView  view;
    eImageType  type;
    uint8_t    *data;
    
}image_t;

// Creates a zero initialized image with the specified cols and rows
image_t *newRGB565Image( const uint32_t cols, const uint32_t rows );

void copy_framebuffer_to_rgb656Image(const camera_fb_t* bf, image_t* image);

#endif // _OPERATORS_H_

#ifdef __cplusplus
 }
#endif
