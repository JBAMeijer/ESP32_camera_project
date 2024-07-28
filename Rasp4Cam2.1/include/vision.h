#ifndef VISION_H_
#define VISION_H_

#include "operators.h"
#include "general.h"

#if defined(IMAGE_RES_VGA) && defined(IMAGE_RES_QVGA) && defined(IMAGE_RES_QQVGA)
#error Only a single image resolution is supported
#endif

#if (!defined(IMAGE_RES_VGA)) && (!defined(IMAGE_RES_QVGA)) && (!defined(IMAGE_RES_QQVGA)) 
#error No image resolution selected
#endif 

#ifdef IMAGE_RES_VGA
#define IMAGE_WIDTH  (640)
#define IMAGE_HEIGHT (480)
#endif

#ifdef IMAGE_RES_QVGA
#define IMAGE_WIDTH  (320)
#define IMAGE_HEIGHT (240)
#endif

#ifdef IMAGE_RES_QQVGA
#define IMAGE_WIDTH  (160)
#define IMAGE_HEIGHT (120)
#endif


#endif // VISION_H_
