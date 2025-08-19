#include "cam.h"
#include "vision.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

typedef struct {
	void* start;
	size_t length;
} MMapBuf;


static MMapBuf* mmaps = NULL;
static s32 mmap_count = 0;
static s32 bytes_per_line = 0;
static s32 size_image = 0;

s32 fd;

s32 width;
s32 height;
s32 bpp;
s32 len;

struct v4l2_requestbuffers bufrequest;
//struct v4l2_buffer bufferinfo;
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

    bytes_per_line = format.fmt.pix.bytesperline;
    size_image = format.fmt.pix.sizeimage;

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
	
	// request buffers
	memset(&bufrequest, 0, sizeof(bufrequest));
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = 4; // atleast 3
    
    if(ioctl(fd, VIDIOC_REQBUFS, &bufrequest) < 0){
		perror("VIDIOC_REQBUFS");
		return(-1);
    }
    
	// check if we got atleast 3
    if (bufrequest.count < 3) {
		fprintf(stderr, "Driver provided < 3 buffers\n");
		return -1;
	}
	
	fprintf(stdout, "Driver provided %d buffers!\n", bufrequest.count);

    mmap_count = bufrequest.count;
    mmaps = calloc(mmap_count, sizeof(MMapBuf));
    if(!mmaps) {
		perror("calloc");
		return -1;
	}
    
    for (s32 i = 0; i < mmap_count; ++i){
		struct v4l2_buffer b = {0};
		b.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		b.memory = V4L2_MEMORY_MMAP;
		b.index = i;
		
		if(ioctl(fd, VIDIOC_QUERYBUF, &b)) {
			perror("VIDIOC_QUERYBUF");
			return(-1);
		}
		
		mmaps[i].length = b.length;
		mmaps[i].start = mmap(
			NULL,
			b.length,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd,
			b.m.offset
		);
		
		if(mmaps[i].start == MAP_FAILED) {
			perror("mmap");
			return(-1);
		}
    
		memset(mmaps[i].start, 0, b.length);
		
		if(ioctl(fd, VIDIOC_QBUF, &b) < 0) {
			perror("VIDIOC_QBUF(init)");
			return (-1);
	    }
	}
    
    
    //~ memset(&bufferinfo, 0, sizeof(bufferinfo));
    
    //~ bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    //~ bufferinfo.memory = V4L2_MEMORY_MMAP;
    //~ bufferinfo.index = 0;
    
    //~ if(ioctl(fd, VIDIOC_QUERYBUF, &bufferinfo) < 0) {
		//~ perror("VIDIOC_QUERYBUF");
		//~ return(-1);
    //~ }
    
    //~ buffer_start = mmap(
		//~ NULL,
		//~ bufferinfo.length,
		//~ PROT_READ | PROT_WRITE,
		//~ MAP_SHARED,
		//~ fd,
		//~ bufferinfo.m.offset
    //~ );
    
    //~ if(buffer_start == MAP_FAILED) {
		//~ perror("mmap");
		//~ return(-1);
    //~ }
    
    //~ memset(buffer_start, 0, bufferinfo.length);
    
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
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
	fd_set fds; 
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	if (select(fd + 1, &fds, NULL, NULL, NULL) <= 0) {
		perror("select");
		return -1;
	}
	
	struct v4l2_buffer b = {0};
	b.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	b.memory = V4L2_MEMORY_MMAP;
	
	if(ioctl(fd, VIDIOC_DQBUF, &b) < 0) {
		perror("VIDIOC_DQBUF");
		return(-1);
	}
	
	//~ register rgb888_pixel_t *s = (rgb888_pixel_t*)buffer_start;
	//~ register rgb888_pixel_t *d = (rgb888_pixel_t*)image->data;
	
	u8* src = (u8*)mmaps[b.index].start;
	u8* dst = (u8*)image->data;
	
	s32 dst_stride = image->cols * 3;
	s32 src_stride = bytes_per_line > 0 ? bytes_per_line : (width * 3);
	
	for (s32 y = 0; y < image->rows; ++y){
		memcpy(dst + y * dst_stride, src + y * src_stride, width *3);
	}
	
	if(ioctl(fd, VIDIOC_QBUF, &b) < 0) {
		perror("VIDIOC_QBUF");
		return (-1);
	}
	
	return 0;
}

s32 close_cam() {
	if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0 ){
		perror("VIDIOC_STREAMOFF");
		return(-1);
    }
    for (s32 i = 0; i < mmap_count; ++i){
		if(mmaps[i].start && mmaps[i].length) munmap(mmaps[i].start, mmaps[i].length);
	}
	free(mmaps); mmaps = NULL; mmap_count = 0;
    close(fd);
    
    return 0;
}
