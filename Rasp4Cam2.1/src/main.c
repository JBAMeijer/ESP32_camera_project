/*

2024-07-03    <joeypi@raspberrypi>

 * Initial version

*/

//image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;


#include <stdio.h>
#include <string.h>
#include <stdint.h>

#if defined(USE_MEM_MANAGER)
#include "mem_manager.h"
#endif

#include "raylib.h"
#include "pc_interface.h"
#include "benchmark.h"
#include "operators.h"
#include "cam.h"

static int width = 1280;
static int height = 800;

static uint8_t str_buf[50] = {0};
static image_t* cam_image = NULL;
static image_t* image_grey = NULL;

int main(void) {
#if defined(USE_MEM_MANAGER)
    if(mem_manager_init() != 0) {
	printf("Failed mem_manager_init\n");
	return -1;
    }
#endif
    
    pc_wifi_interface_start();
    
    if(start_cam(640, 480) != 0) {
	printf("Failed to start camera");
	return -1;
    }
    cam_image = newRGB888Image(640, 480);
    image_grey = newBasicImage(640, 480);
    
    InitWindow(width, height, "raylib [core] example - basic window");
    SetTargetFPS(30);
    SetTraceLogLevel(LOG_ERROR | LOG_WARNING | LOG_FATAL);
    
    //int texture_pos_x = (width - format.fmt.pix.width * scale) / 2;
    float scale = (width/2.0) / 640;
    
    int texture_pos_y = (height - 480 * scale) / 2;
    benchmark_t b;
    benchmark_t grey_conversion;
    Texture texture;
    while (!WindowShouldClose())
    {
	pc_wifi_interface_update();
	benchmark_start(&b, "Cam");
	
	poll_cam(cam_image);	    
	
	benchmark_stop(&b);

	benchmark_start(&grey_conversion, "To basic conversion");
	convert_to_basic_image(cam_image, image_grey);
	benchmark_stop(&grey_conversion);
	
	Image image;
	image.width = cam_image->cols;
	image.height = cam_image->rows;
	image.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
	image.mipmaps = 1;
	image.data = image_grey->data;
	    
	texture = LoadTextureFromImage(image);
	
	//printf(str_buf);
	pc_wifi_interface_send_benchmark(&b);
	
        BeginDrawing();
            ClearBackground(BLACK);
	    DrawTextureEx(texture, (Vector2){0, texture_pos_y}, 0, scale, WHITE);
	    DrawLine(width/2, 0, width/2, height, WHITE);
	    
	    DrawText("SYSTEM INFO", width/2 + 3, 0, 30, LIGHTGRAY);
	    DrawLine(width/2, 30, width, 30, WHITE);
	    DrawText(TextFormat("Current fps: %d", GetFPS()), width/2 + 3, 30, 20, LIGHTGRAY);
	    
	    DrawLine(width/2, 570, width, 570, WHITE);
	    DrawText("Benchmarks", width/2 + 3, 573, 25, LIGHTGRAY);
	    DrawLine(width/2, 600, width, 600, WHITE);
	    
	    benchmark_tostr(&b, str_buf);
	    DrawText(str_buf, width/2 + 3, 603, 20, LIGHTGRAY);
	    memset(str_buf, 0, 50);
	    benchmark_tostr(&grey_conversion, str_buf);
	    DrawText(str_buf, width/2 + 3, 623, 20, LIGHTGRAY);
	    memset(str_buf, 0, 50);
	    
	EndDrawing();
	
	UnloadTexture(texture);
    }
    CloseWindow();
    pc_wifi_interface_stop();
#if defined(USE_MEM_MANAGER)
    mem_manager_free_blocks();
#endif
    
    return(0);
}
