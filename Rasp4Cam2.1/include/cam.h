#ifndef CAM_H_
#define CAM_H_

#include "operators.h"

s32 start_cam();
s32 poll_cam(image_t*);
s32 close_cam();

#endif // CAM_H_
