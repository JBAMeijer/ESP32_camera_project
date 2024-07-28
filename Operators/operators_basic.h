#ifndef _OPERATORS_BASIC_H_
#define _OPERATORS_BASIC_H_

#include "operators.h"

void delete_basic_image(image_t *img);
void convert_to_basic_image(const image_t *src, image_t *dst);
void copy_basic(const image_t *src, image_t *dst);
void threshold_basic(const image_t *src, image_t *dst, u8 threhold_value);
void contrast_stretch_fast_basic(const image_t *src, image_t *dst);

#endif //_OPERATORS_BASIC_H_
