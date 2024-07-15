#ifndef CAM_H_
#define CAM_H_

#include "operators.h"

int start_cam(int, int);
int poll_cam(image_t*);
int close_cam();

#endif // CAM_H_
