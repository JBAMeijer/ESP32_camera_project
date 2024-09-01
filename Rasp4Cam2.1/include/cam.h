#ifndef CAM_H_
#define CAM_H_

#include <time.h>
#include <linux/videodev2.h>
#include "operators.h"

#define CAM_CONTROL_STORAGE_COUNT 100

typedef struct {
	struct v4l2_queryctrl queried_controls[CAM_CONTROL_STORAGE_COUNT];
	struct v4l2_querymenu queried_menu_controls[CAM_CONTROL_STORAGE_COUNT];
	//struct v4l2_querymenu queried_menu_controls[CAM_CONTROL_STORAGE_COUNT];
	s32 values[CAM_CONTROL_STORAGE_COUNT];
} cam_controls_v4l2;



s32 start_cam();
s32 poll_cam(image_t*);
void enumerate_menu(u32);
cam_controls_v4l2 *query_camera_controls(void);
s32 close_cam();

#endif // CAM_H_
