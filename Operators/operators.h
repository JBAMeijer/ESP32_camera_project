#ifdef __cplusplus
    extern "C" {
#endif

#ifndef _OPERATORS_H_
#define _OPERATORS_H_

#include "stdint.h"

#if !defined(BUILD_FROM_GUI)
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
    IMGTYPE_RGB888 = 3,  // RGB 8-bit per pixel
    IMGTYPE_RGB565 = 4,
    IMGTYPE_JPEG   = 5,
  
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
    int32_t     len;
    eImageView  view;
    eImageType  type;
    uint8_t    *data;
    
}image_t;

// Creates a zero initialized image with the specified cols and rows
image_t *newRGB565Image( const uint32_t cols, const uint32_t rows );

image_t *newJPEGImageFromData(const uint8_t* data, const uint32_t cols, const uint32_t rows, const uint32_t len);

void deleteJPEGImage(image_t* img);

#endif // _OPERATORS_H_

#ifdef __cplusplus
 }
#endif
