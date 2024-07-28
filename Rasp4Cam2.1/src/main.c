/*

2024-07-03    <joeypi@raspberrypi>

 * Initial version

*/

//image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <stdatomic.h>
#include <pigpio.h>

#include "mem_manager.h"

#include "raylib.h"
#include "raygui.h"
#include "pc_interface.h"
#include "benchmark.h"
#include "operators.h"
#include "cam.h"
#include "general.h"
#include "vision.h"

#define INIT_VIEWER_IMAGE(c,r) newRGB888Image(c,r)

PixelFormat get_pixel_format(image_t *image);
void upload_to_image_viewer(image_t *image, const s8 *name);
void set_viewer(void);
void draw_viewerboxes_and_view(void);

void draw_image_info(void);
void draw_processing_controls_window(void);
void draw_acquisition_controls_window(void);

void draw_info_control_window(void);
void upload_to_benchmark_viewer(benchmark_t *benchmark);
void draw_benchmarks(void);

void init_benchmark_viewer_data(void);
void init_processing_control_data(void);

void *change_internal_data_representation(image_t *image);

void send_processing_time_total(double time);

void slider_bar(const s8* name, f32* value, s32 bottom, s32 top);

//~ void copy_queue(void);

s32 vision_setup(void);
void vision_main(void);

// ----------------------------------------------------------------------------
// Main viewer typdefs and data
// ----------------------------------------------------------------------------
typedef struct {
    u32 lowest;
    u32 highest;
    u32 elapsed_time;
    s8 name[30];
} benchmarkViewerData;

typedef enum {
    CONTROL_SLIDERBAR,
} eControlType;

typedef struct {
    f32 val;
    u32 top;
    u32 bottom;
} sliderbarData;

typedef union {
    sliderbarData sliderbar_data;
} uControlData;

typedef struct {
    eControlType type;
    uControlData control_data;
    s8 name[30];
} processingControlData;

typedef enum {
    SIDE_WINDOW_IMAGE_INFO,
    SIDE_WINDOW_PROCESSING_CONTROLS,
    SIDE_WINDOW_ACQUISITION_CONTROLS,
} eImageInfoControlWindow;

static const s32 width = 1280;
static const s32 height = 800;

static const f32 scale = (width/2.0) / 640;
static const s32 texture_pos_y = (height - 480 * scale) / 2;

static s8 str_viewer_buf[100] = {0};
static s32 current_viewer = -1;
static Texture texture;
static eImageInfoControlWindow view_window = SIDE_WINDOW_IMAGE_INFO;

static benchmarkViewerData benchmark_list[10] = {0};

static processingControlData processing_controls_data[10] = {0};

static image_t *image_viewer_images[N_REQUIRED_DATA_BLOCKS];
static s8 image_viewer_image_names[N_REQUIRED_DATA_BLOCKS][30];
static bool image_viewer_location_occupied[N_REQUIRED_DATA_BLOCKS];

static s32 benchmarks_count = 0;

static Rectangle views[10] = {
	(Rectangle){0,   0, 50, 50},
	(Rectangle){50,  0, 50, 50},
	(Rectangle){100, 0, 50, 50},
	(Rectangle){150, 0, 50, 50},
	(Rectangle){200, 0, 50, 50},
	(Rectangle){250, 0, 50, 50},
	(Rectangle){300, 0, 50, 50},
	(Rectangle){350, 0, 50, 50},
	(Rectangle){400, 0, 50, 50},
	(Rectangle){450, 0, 50, 50},
};

static double processing_time = 0;

//static u32 image_info_text_len = 0;

//static const Rectangle rec = (Rectangle){ width/2 - 1, 298, width/2 + 2, 30 };
static const Rectangle rec_image_info = (Rectangle){ width/2 - 1, 298, 192, 30 };
static const Rectangle rec_processing_controls = (Rectangle){ rec_image_info.x + rec_image_info.width - 1, 298, 208, 30 };
static const Rectangle rec_acquisition_controls = (Rectangle){ rec_processing_controls.x + rec_processing_controls.width - 1, 298, 186, 30 };

//~ static Rectangle rec;
//~ static Rectangle rec2;

// ----------------------------------------------------------------------------
// Vision_main typdefs and data
// ----------------------------------------------------------------------------
static image_t *cam_image;
static image_t *image_grey;
static image_t *contrast_stretch_image;
static image_t *threshold_image;

s32 main(void) {
    if(gpioInitialise() < 0) {
	fprintf(stderr, "Failed to initialize pigpio\n");
    }
    
    if(mem_manager_init() != 0) {
	printf("Failed mem_manager_init!\n");
	return (-1);
    }
    
    if(start_cam() != 0) {
	printf("Failed to start camera!\n");
	return (-1);
    }
    
    if(vision_setup() != 0) {
	fprintf(stderr, "Failed vision setup!\n");
	return (-1);
    }
    
    InitWindow(width, height, "raylib [core] example - basic window");
    SetTargetFPS(30);
    SetTraceLogLevel(LOG_ERROR | LOG_WARNING | LOG_FATAL);
    
    init_benchmark_viewer_data();
    init_processing_control_data();
    
    while (!WindowShouldClose())
    {
	vision_main();
	
	u8 free_blocks = mem_manager_free_blocks();
	
	float block_per = (N_REQUIRED_DATA_BLOCKS - (float)free_blocks) / N_REQUIRED_DATA_BLOCKS;
	
        BeginDrawing();
            ClearBackground(BLACK);
	    
	    draw_viewerboxes_and_view();
	    DrawLine(width/2, 0, width/2, height, WHITE);
	    
	    DrawText("SYSTEM INFO", width/2 + 3, 0, 30, LIGHTGRAY);
	    DrawLine(width/2, 30, width, 30, WHITE);
	    DrawText(TextFormat("Current viewer fps: %d", GetFPS()), width/2 + 3, 30, 20, LIGHTGRAY);
	    
	    if(processing_time != 0)
	    {
		DrawText(TextFormat("Current processing fps: %d", (u32)(1/processing_time)),
		    width/2 + 3, 50, 20, LIGHTGRAY);
		DrawText(TextFormat("Current processing time: %0.1f ms", processing_time * 1000),
		    width/2 + 3, 70, 20, LIGHTGRAY);
	    }
	    
	    DrawText(TextFormat("Total data block memory: %0.3f kB", BLOCK_SIZE*N_REQUIRED_DATA_BLOCKS/1000.0),
		width/2 + 3, 100, 20, LIGHTGRAY);
		
	    DrawText(TextFormat("Used blocks: %d", N_REQUIRED_DATA_BLOCKS - free_blocks),
		width/2 + 3, 130, 20, LIGHTGRAY);
	    
	    DrawText(TextFormat("Free blocks: %d", free_blocks),
		width/2 + 3, 150, 20, LIGHTGRAY);
		
	    DrawText("Used memory", width/2 + 3, 175, 20, LIGHTGRAY);
	    DrawRectangle(width/2 + 143, 175, 20, 20, YELLOW);
	    
	    DrawText("Free memory", width/2 + 203, 175, 20, LIGHTGRAY);
	    DrawRectangle(width/2 + 343, 175, 20, 20, BLUE);
	    
	    DrawRectangle(width/2 + 10, 205, width/2 - 20, 25, BLUE);
	    DrawRectangle(width/2 + 10, 205, (width/2 - 20) * block_per, 25, YELLOW);
	    
	    draw_info_control_window();
	    
	    DrawLine(width/2, 570, width, 570, WHITE);
	    DrawText("Benchmarks", width/2 + 3, 573, 25, LIGHTGRAY);
	    DrawLine(width/2, 600, width, 600, WHITE);
	    
	    draw_benchmarks();
	    
	EndDrawing();
    }
    CloseWindow();
    gpioTerminate();
    
    return(0);
}

s32 vision_setup(void) {
    cam_image = newRGB888Image(IMAGE_WIDTH, IMAGE_HEIGHT);
    image_grey = newBasicImage(IMAGE_WIDTH, IMAGE_HEIGHT);
    contrast_stretch_image = newBasicImage(IMAGE_WIDTH, IMAGE_HEIGHT);
    threshold_image = newBasicImage(IMAGE_WIDTH, IMAGE_HEIGHT);
    
    if(threshold_image == NULL)
    {
	fprintf(stderr, "Failed to allocate required memory for images!\n");
	return(-1);
    }
	
    return(0);
}

void vision_main(void) {
    benchmark_t benchmark;
    clock_t start, end;
    double total;

    start = clock();
    benchmark_start(&benchmark, "Cam");
    poll_cam(cam_image);	    
    benchmark_stop(&benchmark);
	
    upload_to_benchmark_viewer(&benchmark);
    upload_to_image_viewer(cam_image, "Cam image");

    benchmark_start(&benchmark, "To basic conversion");
    convert_image(cam_image, image_grey);
    benchmark_stop(&benchmark);
	
    upload_to_image_viewer(image_grey, "RGB888 to Basic conversion");
    upload_to_benchmark_viewer(&benchmark);
    
    benchmark_start(&benchmark, "Contrast stretch fast");
    contrast_stretch_fast(image_grey, contrast_stretch_image);
    benchmark_stop(&benchmark);
	
    upload_to_image_viewer(contrast_stretch_image, "Contrast stretch fast");
    upload_to_benchmark_viewer(&benchmark);
    
    static f32 threshold_value = 128;
    slider_bar("Threshold", &threshold_value, 0, 255);
    
    benchmark_start(&benchmark, "Threshold");
    threshold(contrast_stretch_image, threshold_image, (u8)threshold_value);
    benchmark_stop(&benchmark);
	
    upload_to_image_viewer(threshold_image, "Threshold");
    upload_to_benchmark_viewer(&benchmark);
    
    end = clock();
    total = (double)(end - start)/CLOCKS_PER_SEC;
	
    send_processing_time_total(total);
}

void init_processing_control_data(void) {
    for(u32 i = 0; i < 10; i++) {
	processing_controls_data[i].name[29] = '\0';
    }
}

void init_benchmark_viewer_data(void) {
    for(u32 i = 0; i < 10; i++) {
	benchmark_list[i].lowest = 9999999;
	benchmark_list[i].highest = 0;
    }
}

void draw_viewerboxes_and_view(void) {
    if(current_viewer == -1) {
    	for(s32 i = 0; i < N_REQUIRED_DATA_BLOCKS; i++) {
	    if(image_viewer_images[i] != NULL){
		current_viewer = i;
		break;
	    }
	}
    }
    
    Vector2 mouse_pos = GetMousePosition();	
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
	for(s32 i = 0; i < N_REQUIRED_DATA_BLOCKS; i++) {
	    if(image_viewer_images[i] != NULL && CheckCollisionPointRec(mouse_pos, views[i])){
		current_viewer = i;
		break;
	    }
	}
    }
    
    for(s32 i = 0; i < N_REQUIRED_DATA_BLOCKS; i++) {	
	if(image_viewer_images[i] != NULL) {
	    if(current_viewer == i) {
		DrawText(TextFormat("%d", i), views[i].x + 15, views[i].y + 5, 40, BLUE);
		DrawRectangleLinesEx(views[i], 1, BLUE);
	    } else if(CheckCollisionPointRec(mouse_pos, views[i])) {
		DrawText(TextFormat("%d", i), views[i].x + 15, views[i].y + 5, 40, YELLOW);
		DrawRectangleLinesEx(views[i], 1, YELLOW);
	    } else {
		DrawText(TextFormat("%d", i), views[i].x + 15, views[i].y + 5, 40, LIGHTGRAY);
		DrawRectangleLinesEx(views[i], 1, LIGHTGRAY);
	    }
	}
    }
    
    if(current_viewer != -1) {
	Image raylib_image;
	raylib_image.width = image_viewer_images[current_viewer]->cols;
	raylib_image.height = image_viewer_images[current_viewer]->rows;
	raylib_image.format = get_pixel_format(image_viewer_images[current_viewer]);
	raylib_image.mipmaps = 1;
			    
	if(image_viewer_images[current_viewer]->view == IMGVIEW_CLIP) 
	    raylib_image.data = image_viewer_images[current_viewer]->data;
	else if(image_viewer_images[current_viewer]->view == IMGVIEW_BINARY)
	    raylib_image.data = change_internal_data_representation(image_viewer_images[current_viewer]);
			    
	memset(str_viewer_buf, 0, 100);
	strncpy(str_viewer_buf, image_viewer_image_names[current_viewer], 30);
	    
	UnloadTexture(texture);
	texture = LoadTextureFromImage(raylib_image);
	image_viewer_location_occupied[current_viewer] = false;
    }
    
    s32 measured_text_length = MeasureText(str_viewer_buf, 30);
    s32 position_x_text = (width/2 - measured_text_length) / 2;
    
    DrawText(str_viewer_buf, position_x_text, texture_pos_y - 30, 30, LIGHTGRAY);
    DrawTextureEx(texture, (Vector2){0, texture_pos_y}, 0, scale, WHITE);

}


void draw_info_control_window(void) {
    Vector2 mouse_pos = GetMousePosition();
    
    if(view_window == SIDE_WINDOW_IMAGE_INFO) {
    	DrawRectangleLinesEx(rec_image_info, 1, BLUE);
        DrawText("IMAGE INFO", rec_image_info.x + 3, 300, 30, BLUE);
    } else if(CheckCollisionPointRec(mouse_pos, rec_image_info)) {
	DrawRectangleLinesEx(rec_image_info, 1, YELLOW);
        DrawText("IMAGE INFO", rec_image_info.x + 3, 300, 30, YELLOW);
	if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) view_window = SIDE_WINDOW_IMAGE_INFO;
    } else {
        DrawRectangleLinesEx(rec_image_info, 1, WHITE);
        DrawText("IMAGE INFO", rec_image_info.x + 3, 300, 30, LIGHTGRAY);
    }
    
    if(view_window == SIDE_WINDOW_PROCESSING_CONTROLS) {
	DrawRectangleLinesEx(rec_processing_controls, 1, BLUE);
	DrawText("PRO CNTRLS", rec_processing_controls.x + 3, 300, 30, BLUE);
    } else if(CheckCollisionPointRec(mouse_pos, rec_processing_controls)) {
	DrawRectangleLinesEx(rec_processing_controls, 1, YELLOW);
	DrawText("PRO CNTRLS", rec_processing_controls.x + 3, 300, 30, YELLOW);
	if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) view_window = SIDE_WINDOW_PROCESSING_CONTROLS;
    } else {
	DrawRectangleLinesEx(rec_processing_controls, 1, WHITE);
	DrawText("PRO CNTRLS", rec_processing_controls.x + 3, 300, 30, WHITE);
    }
    
    if(view_window == SIDE_WINDOW_ACQUISITION_CONTROLS) {
	DrawRectangleLinesEx(rec_acquisition_controls, 1, BLUE);
	DrawText("AQ CNTRLS", rec_acquisition_controls.x + 3, 300, 30, BLUE);
    } else if(CheckCollisionPointRec(mouse_pos, rec_acquisition_controls)) {
	DrawRectangleLinesEx(rec_acquisition_controls, 1, YELLOW);
	DrawText("AQ CNTRLS", rec_acquisition_controls.x + 3, 300, 30, YELLOW);
	if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) view_window = SIDE_WINDOW_ACQUISITION_CONTROLS;
    } else {
	DrawRectangleLinesEx(rec_acquisition_controls, 1, WHITE);
	DrawText("AQ CNTRLS", rec_acquisition_controls.x + 3, 300, 30, WHITE);
    }

    DrawLine(width/2, 329, width, 329, WHITE);
    
    switch(view_window) {
	case SIDE_WINDOW_IMAGE_INFO:
	    draw_image_info();
	    break;
	case SIDE_WINDOW_PROCESSING_CONTROLS:
	    draw_processing_controls_window();
	    break;
	case SIDE_WINDOW_ACQUISITION_CONTROLS:
	    break;
    }
}

void draw_image_info(void) {
    if(current_viewer != -1) {
	image_t *current_viewer_image = image_viewer_images[current_viewer];
	s32 measured_text_length = MeasureText("Image name: ", 20);
	
	DrawText("Image name: ", width/2 + 3, 330, 20, LIGHTGRAY);
	DrawText(TextFormat("%s", str_viewer_buf), width/2 + (3 + measured_text_length), 330, 20, LIGHTGRAY);
	
	DrawText("Columns: ", width/2 + 3, 350, 20, LIGHTGRAY);
	DrawText(TextFormat("%d", current_viewer_image->cols), width/2 + (3 + measured_text_length), 350, 20, LIGHTGRAY);
	
	DrawText("Rows: ", width/2 + 3, 370, 20, LIGHTGRAY);
	DrawText(TextFormat("%d", current_viewer_image->rows), width/2 + (3 + measured_text_length), 370, 20, LIGHTGRAY);
	
	DrawText("Type: ", width/2 + 3, 390, 20, LIGHTGRAY);
	DrawText(TextFormat("%s", IMGTYPE_NAMES[current_viewer_image->type]), width/2 + (3 + measured_text_length), 390, 20, LIGHTGRAY);
	
	DrawText("View: ", width/2 + 3, 410, 20, LIGHTGRAY);
	DrawText(TextFormat("%s", IMGVIEW_NAMES[current_viewer_image->view]), width/2 + (3 + measured_text_length), 410, 20, LIGHTGRAY);
    }
}

void draw_processing_controls_window(void) {
    s32 position = 330;
    for(u32 i = 0; i < 10; i++) {
	if(processing_controls_data[i].name[0] != '\0') {
	    if(processing_controls_data[i].type == CONTROL_SLIDERBAR) {
		DrawText(processing_controls_data[i].name, width/2 + 3, position, 20, LIGHTGRAY);
		GuiSlider((Rectangle){width - 252, position, 150, 20}, NULL, NULL, 
		    &processing_controls_data[i].control_data.sliderbar_data.val, 
		    processing_controls_data[i].control_data.sliderbar_data.bottom, 
		    processing_controls_data[i].control_data.sliderbar_data.top
		);
		DrawText(TextFormat("%0.0f", processing_controls_data[i].control_data.sliderbar_data.val), width - 70, position, 20, LIGHTGRAY);
		position += 20;
	    }
	}
    }
}

void send_processing_time_total(double time) {
    processing_time = time;
}

void draw_benchmarks(void) {
    for(u32 index = 0; index < benchmarks_count; index++) {
	DrawText(benchmark_list[index].name, width/2 + 3, 603 + 40 * index, 20, GREEN);
	
	DrawText(TextFormat("Elapsed: %d, lowest: %d, highest: %d.", 
	benchmark_list[index].elapsed_time, benchmark_list[index].lowest, benchmark_list[index].highest),
	    width/2 + 3, 623 + 40 * index, 20, LIGHTGRAY);
    }
    
    benchmarks_count=0;
}

void slider_bar(const s8* name, f32* value, s32 bottom, s32 top) {
    bool found = false;
    for(u32 i = 0; i < 10; i++) {
	if(strncmp(processing_controls_data[i].name, name, 30) == 0) {
	    s32 tmp = *value;
	    *value = processing_controls_data[i].control_data.sliderbar_data.val;
	    processing_controls_data[i].control_data.sliderbar_data.val = tmp;
	    processing_controls_data[i].control_data.sliderbar_data.top = top;
	    processing_controls_data[i].control_data.sliderbar_data.bottom = bottom;
	    found = true;
	    break;
	}
    }
    
    if(!found) {
	for(u32 i = 0; i < 10; i++) {
	    if(processing_controls_data[i].name[0] == '\0') {
		strncpy(processing_controls_data[i].name, name, 30);
		image_viewer_image_names[i][29] = '\0';
		processing_controls_data[i].type = CONTROL_SLIDERBAR;
		processing_controls_data[i].control_data.sliderbar_data.val = *value;
		processing_controls_data[i].control_data.sliderbar_data.top = top;
		processing_controls_data[i].control_data.sliderbar_data.bottom = bottom;
		break;
	    }
	}
    }
}


void upload_to_image_viewer(image_t *image, const s8 *name) {
    for(u32 i = 0; i < N_REQUIRED_DATA_BLOCKS; i++) {
	if(image_viewer_images[i] == NULL) {
	    image_viewer_images[i] = image;
	    strncpy(image_viewer_image_names[i], name, 30);
	    image_viewer_image_names[i][29] = '\0';
	    break;
	} else if(image_viewer_images[i] == image) {
	    break;
	}
    }
}

void *change_internal_data_representation(image_t *image) {
    register s32 i = image->rows * image->cols / 4;
    register u32 *pixelsrc = (u32*)image->data;
    register u32 *pixeldst = (u32*)image->data;
    register u32 tmp = 0;
	
    while(i-- > 0) {
	tmp = *pixelsrc;
	
	*pixeldst  = (((tmp & 0xFF000000) >> 24)) * 0xFF000000;
	*pixeldst |= (((tmp & 0xFF0000) >> 16)) * 0xFF0000;
	*pixeldst |= (((tmp & 0xFF00) >> 8)) * 0xFF00;
	*pixeldst |= (((tmp & 0xFF))) * 0xFF;
		
	pixeldst++;
	pixelsrc++;
    }
    
    return image->data;
}

void upload_to_benchmark_viewer(benchmark_t *benchmark) {
    benchmark_list[benchmarks_count].elapsed_time = benchmark->stop - benchmark->start;
    if(benchmark_list[benchmarks_count].elapsed_time > benchmark_list[benchmarks_count].highest) 
	benchmark_list[benchmarks_count].highest = benchmark_list[benchmarks_count].elapsed_time;
    else if(benchmark_list[benchmarks_count].elapsed_time < benchmark_list[benchmarks_count].lowest) 
	benchmark_list[benchmarks_count].lowest = benchmark_list[benchmarks_count].elapsed_time;
    
    strncpy(benchmark_list[benchmarks_count].name, benchmark->name, 30);
    benchmarks_count++;
}

PixelFormat get_pixel_format(image_t *image) {
    switch(image->type) {
	case IMGTYPE_BASIC:
	    return PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
	case IMGTYPE_RGB888:
	    return PIXELFORMAT_UNCOMPRESSED_R8G8B8;
	default:
	    fprintf(stderr, "unkown case");
    }
    
    return -1;
}
