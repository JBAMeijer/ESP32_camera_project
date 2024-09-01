#include "cam.h"
#include "vision.h"

#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

s32 fd;

s32 width;
s32 height;
s32 bpp;
s32 len;

struct v4l2_requestbuffers bufrequest;
struct v4l2_buffer bufferinfo;
s32 type;
void *buffer_start = NULL;

static cam_controls_v4l2 cam_controls;

struct v4l2_queryctrl queryctrl;
struct v4l2_querymenu querymenu;

static u32 query_name_index = 0;

s32 start_cam() {

    if((fd = open("/dev/video0", O_RDWR)) < 0) {
		perror("open");
		return(-1);
    }
    
    struct v4l2_capability cap;
    if(ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
		perror("VIDIOC_QUERYCAP");
		return(-1);
    }
    
    if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "The device does not handle single-planar video capture.\n");
		return(-1);
    }
    
    struct v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
    format.fmt.pix.width = IMAGE_WIDTH;
    format.fmt.pix.height = IMAGE_HEIGHT;
    
    if(ioctl(fd, VIDIOC_S_FMT, &format) < 0) {
		perror("VIDIOC_S_FMT");
		return(-1);
    }
    
    if(format.fmt.pix.width != IMAGE_WIDTH || 
		format.fmt.pix.height != IMAGE_HEIGHT) {
		fprintf(stderr, "Failed to set the desired width or height!\n");
		return(-1);
	}
    
    width = format.fmt.pix.width;
    height = format.fmt.pix.height;
    bpp = 3;
    len = width * height * bpp;
    
    struct v4l2_streamparm streamparm;
    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd, VIDIOC_G_PARM, &streamparm) < 0) {
		perror("VIDIOC_G_PARM");
		return(-1);
    }
    
    if(!(streamparm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)) {
		fprintf(stderr, "The device does not handle time_per_frame.\n");
		return(-1);
    }
    
    struct v4l2_control control;
    control.id = V4L2_CID_EXPOSURE_AUTO;
    control.value = V4L2_EXPOSURE_AUTO;
    
    printf("control.id: %d\n", control.id);
    printf("control.value: %d\n", control.value);
    
    if(ioctl(fd, VIDIOC_S_CTRL, &control) < 0){
		perror("VIDIOC_S_CTRL");
		return(-1);
    }
    
    printf("control.value after: %d\n", control.value);
    
    control.id = 0x009a0902;
    control.value = 10;
    
    printf("control.value: %d\n", control.value);
    
    if(ioctl(fd, VIDIOC_S_CTRL, &control) < 0){
		perror("VIDIOC_S_CTRL");
		return(-1);
    }
    
    printf("control.value after: %d\n", control.value);
    
    control.id = 0x009a0918;
    control.value = 0;
    
    printf("control.value: %d\n", control.value);
    
    if(ioctl(fd, VIDIOC_S_CTRL, &control) < 0){
		perror("VIDIOC_S_CTRL");
		return(-1);
    }
    
    printf("control.value after: %d\n", control.value);
    
    control.id = 0x00980922;
    control.value = 180;
    
    printf("control.value rotate: %d\n", control.value);
    
    if(ioctl(fd, VIDIOC_S_CTRL, &control) < 0){
		perror("VIDIOC_S_CTRL");
		return(-1);
    }
    
    printf("control.value rotate after: %d\n", control.value);

    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = 1;
    
    if(ioctl(fd, VIDIOC_REQBUFS, &bufrequest) < 0){
		perror("VIDIOC_REQBUFS");
		return(-1);
    }

    memset(&bufferinfo, 0, sizeof(bufferinfo));
    
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;
    
    if(ioctl(fd, VIDIOC_QUERYBUF, &bufferinfo) < 0) {
		perror("VIDIOC_QUERYBUF");
		return(-1);
    }
    
    buffer_start = mmap(
		NULL,
		bufferinfo.length,
		PROT_READ | PROT_WRITE,
		MAP_SHARED,
		fd,
		bufferinfo.m.offset
    );
    
    if(buffer_start == MAP_FAILED) {
		perror("mmap");
		return(-1);
    }
    
    memset(buffer_start, 0, bufferinfo.length);
    
    type = bufferinfo.type;
    if(ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
		perror("VIDIOC_STREAMON");
		return -1;
    }
    
    return 0;
}

void enumerate_menu(u32 id) {
    printf("  Menu items:\n");

    memset(&querymenu, 0, sizeof(querymenu));
    querymenu.id = id;

    for (querymenu.index = queryctrl.minimum;
         querymenu.index <= queryctrl.maximum;
         querymenu.index++) {
		s32 result = ioctl(fd, VIDIOC_QUERYMENU, &querymenu);
        if (result == 0) {
			printf("  %s\n", querymenu.name);
			//strcat()
			memcpy(&cam_controls.queried_menu_controls[query_name_index], &querymenu, sizeof(querymenu));
			//cam_controls.queried_menu_controls[querymenu]
			//strncpy(cam_controls.queried_menu_control_names[query_name_index], (s8 *)querymenu.name, 32);
            query_name_index++;
        } else {
			if(errno == EINVAL)
				printf("Strange param: EINVAL index=%d\n", querymenu.index);
		}
        
    }
}

cam_controls_v4l2 *query_camera_controls(void) {
	memset(&cam_controls, 0, sizeof(cam_controls));
	memset(&queryctrl, 0, sizeof(queryctrl));
	   
	struct v4l2_control control;
	memset(&control, 0, sizeof(control));
	
	queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
	
	s32 i = 0;
	
	while (0 == ioctl(fd, VIDIOC_QUERYCTRL, &queryctrl)) {
		if (!(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)) {
			printf("Control %s : ", queryctrl.name);
			printf("min=%d max=%d step=%d type=%d default=%d flags=%04x ", 
				queryctrl.minimum, queryctrl.maximum, queryctrl.step, queryctrl.type, queryctrl.default_value, queryctrl.flags);

			if (queryctrl.type == V4L2_CTRL_TYPE_MENU) {
				enumerate_menu(queryctrl.id);
			}
			
			if(i < CAM_CONTROL_STORAGE_COUNT) {
				if (queryctrl.type == V4L2_CTRL_TYPE_INTEGER || 
					queryctrl.type == V4L2_CTRL_TYPE_BOOLEAN ||
					queryctrl.type == V4L2_CTRL_TYPE_MENU 	 ||
					queryctrl.type == V4L2_CTRL_TYPE_INTEGER_MENU) {
					
					memcpy(&cam_controls.queried_controls[i], &queryctrl, sizeof(queryctrl));
					control.id = queryctrl.id;
					if (0 == ioctl(fd, VIDIOC_G_CTRL, &control)) {
						cam_controls.values[i] = control.value;
					}
					
					printf("current value=%d\n", control.value);
					
					i++;
				} else {
					printf("\n");
				}
			}
		}

		queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
	}
	if (errno != EINVAL) {
		perror("VIDIOC_QUERYCTRL");
		query_name_index = 0;
		return(NULL);
	}
	
	query_name_index = 0;
	return(&cam_controls);
}

s32 poll_cam(image_t *image) {
	for(s32 i = 0; i < bufrequest.count; i++) {
	    if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0) {
			perror("VIDIOC_QBUF");
			return (-1);
	    }
	    
	    s32 i = image->cols * image->rows;
	    register rgb888_pixel_t *s = (rgb888_pixel_t*)buffer_start;
		register rgb888_pixel_t *d = (rgb888_pixel_t*)image->data;
	    
	    while(i--)
			*d++ = *s++;
	    
	    if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0) {
			perror("VIDIOC_DQBUF");
			return(-1);
	    }
	}
	
	return 0;
}

s32 close_cam() {
	if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0 ){
		perror("VIDIOC_STREAMOFF");
		return(-1);
    }
    close(fd);
    
    return 0;
}
