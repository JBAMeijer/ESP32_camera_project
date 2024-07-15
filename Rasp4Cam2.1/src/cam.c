#include "cam.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <linux/videodev2.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

int fd;

int width;
int height;
int bpp;
int len;

struct v4l2_requestbuffers bufrequest;
struct v4l2_buffer bufferinfo;
int type;
void* buffer_start = NULL;

int start_cam(int width, int height) {

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
    format.fmt.pix.width = width;
    format.fmt.pix.height = height;
    
    if(ioctl(fd, VIDIOC_S_FMT, &format) < 0) {
		perror("VIDIOC_S_FMT");
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
    
    /*struct v4l2_frmivalenum frameinterval;
    frameinterval.pixel_format = V4L2_PIX_FMT_RGB24;
    frameinterval.width = 1920;
    frameinterval.height = 1080;
    frameinterval.index = 0;
    if(ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frameinterval) < 0) {
	perror("VIDIOC_G_PARM");
	return(-1);
    }
    
    printf("Frame interval: %d\n", frameinterval.type);
    printf("Frame min n: %d\n", frameinterval.stepwise.min.numerator);
    printf("Frame min d: %d\n", frameinterval.stepwise.min.denominator);
    printf("Frame max n: %d\n", frameinterval.stepwise.max.numerator);
    printf("Frame max d: %d\n", frameinterval.stepwise.max.denominator);
    printf("Frame step: %d\n", frameinterval.stepwise.step.numerator);
    printf("Frame step: %d\n", frameinterval.stepwise.step.denominator);
    
    struct v4l2_fract fract;
    fract.numerator = 1;
    fract.denominator = 7;
    streamparm.parm.capture.timeperframe = fract;
    if(ioctl(fd, VIDIOC_S_PARM, &streamparm) < 0) {
	perror("VIDIOC_S_PARM");
	return(-1);
    }
    
    printf("time per frame numerator: %d\n", streamparm.parm.capture.timeperframe.numerator);
    printf("time per frame denominator: %d\n", streamparm.parm.capture.timeperframe.denominator);
*/

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

int poll_cam(image_t* image) {
	for(int i = 0; i < bufrequest.count; i++) {
	    if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0) {
			perror("VIDIOC_QBUF");
			return (-1);
	    }
	    
	    int i = image->cols * image->rows;
	    register rgb888_pixel_t* s = (rgb888_pixel_t*)buffer_start;
		register rgb888_pixel_t* d = (rgb888_pixel_t*)image->data;
	    
	    while(i--)
			*d++ = *s++;
	    
	    if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0) {
			perror("VIDIOC_DQBUF");
			return(-1);
	    }
	}
	
	return 0;
}

int close_cam() {
	if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0 ){
		perror("VIDIOC_STREAMOFF");
		return(-1);
    }
    close(fd);
    
    return 0;
}
