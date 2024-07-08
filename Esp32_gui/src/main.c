#include <stdlib.h> 
#include <string.h>

#include "raylib.h"
#include "raymath.h"
#include "raygui.h"
#include "general.h"
#include "benchmark.h"
#include "device_wifi_interface.h"

#include "style_dark.h"

void device_version_callback(s32, s32, s32);
void benchmark_callback(benchmark_t);
void image_callback(image_t*);

void set_benchmarks_names_to_zero(void);

const s32 screenWidth = 1280;
const s32 screenHeight = 800;
const u32 benchmark_list_count = 50;

static u32 device_version[3] = {0, 0, 0};

static benchmark_t benchmarks[benchmark_list_count];
static u8 benchmark_str_buffer[100] = {0};
// static u8 data_temp[240*240*1];

static Texture2D texture;

int main(void) {
    InitWindow(screenWidth, screenHeight, "esp32 gui");
    SetTargetFPS(60);

    GuiLoadStyleDark();
    // image_t* image = newRGB565Image(320, 240);
    // register u32 i = image->cols * image->rows;
    // register rgb565_pixel_t *s = (rgb565_pixel_t *)image->data;
    
    // while(i-- > 0)
    //     *s++ = 0b1111100000000000;

    // FILE* fptr;
    // if((fptr = fopen("grayimage.data", "rb")) == NULL){
    //     printf("Error! opening file");
    //     return (-1);
    // }

    // fread(data_temp, sizeof(u8), 240*240*1, fptr);

    // fclose(fptr);

    // register u32 i = 320*240;
    // register rgb565_pixel_t *s = (rgb565_pixel_t *)data_temp;
    
    // while(i-- > 0) {
    //     *s++ = 0b1111100000000000;
    // }
    // Image image_local;
    // image_local.width = 240;
    // image_local.height = 240;
    // image_local.format = PIXEL;
    // image_local.mipmaps = 1;
    // image_local.data = data_temp;
    // Texture2D texture_test = LoadTextureFromImage(image_local);
    //texture = LoadTexture("resources/romulus.png");
    //Font font = LoadFontEx("resources/romulus.png", 32, 0, 250);
    // Anchor definitions  
    Vector2 anchor01 = { 0, 0 };
    Vector2 anchor02 = { 0, 40 };
    Vector2 anchor03 = { 0, screenHeight - 20 };
    
    // Rectangle definitions
    Rectangle top_bar_panel = { anchor01.x, anchor01.y, screenWidth, 40 };
    Rectangle side_bar_panel = { anchor02.x, anchor02.y, 200, .height = screenHeight - top_bar_panel.height - 20 };
    Rectangle bottom_bar_panel = { anchor03.x, anchor03.y, screenWidth, 20 };
    Rectangle benchmark_scroll_panel = { 200, 600, 1080, 182 };

    // control variable definitions
    Rectangle scroll_panel_view = { 0, 0, 0, 0 };
    Rectangle scroll_panel_content = { benchmark_scroll_panel.x, benchmark_scroll_panel.y, benchmark_scroll_panel.width - 20, 0};
    Vector2 scroll_panel_scroll_offset = { 0, 0 };

    while(!WindowShouldClose()) {
        device_wifi_interface_update();

        Vector2 mouse_postion = GetMousePosition();

        BeginDrawing();
        DrawTexture(texture, side_bar_panel.width, 40, WHITE);
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        { // Draw Top panel
            GuiDummyRec(top_bar_panel, NULL);
        }
        { // Draw side panel
            GuiPanel(side_bar_panel, NULL);
            GuiGroupBox((Rectangle){anchor02.x + 6, anchor02.y + 14, side_bar_panel.width - 12, 184}, "Device connection");
            if(connected()) GuiDisable();
            if(GuiButton((Rectangle) {anchor02.x + 16, anchor02.y + 32, side_bar_panel.width - 32, 20}, "Connect")) open_connection(device_version_callback, benchmark_callback, image_callback);
            if(connected()) GuiEnable();
            
            if(!connected()) GuiDisable();
            if(GuiButton((Rectangle) {anchor02.x + 16, anchor02.y + 62, side_bar_panel.width - 32, 20}, "Disconnect")) close_connection();
            if(!connected()) GuiEnable();

            GuiLine((Rectangle){anchor02.x + 8, anchor02.y + 88, 184, 12}, NULL);
            if(!connected()) GuiLabel((Rectangle){anchor02.x + 16, anchor02.y + 96, 168, 24}, TextFormat("Device version: x.x.x"));
            if(connected()) GuiLabel((Rectangle){anchor02.x + 16, anchor02.y + 96, 168, 24}, TextFormat("Device version: %d.%d.%d", device_version[0], device_version[1], device_version[2]));

            GuiGroupBox((Rectangle){anchor02.x + 6, anchor02.y + 206, side_bar_panel.width - 12, 170}, "Other info");
            GuiLabel((Rectangle){anchor02.x + 12, anchor02.y + 214, side_bar_panel.width - 24, 24}, TextFormat("Mouse pos - x:%.0f, y:%.0f", mouse_postion.x, mouse_postion.y));

        }
        { // Draw benchmark panel
            GuiScrollPanel(benchmark_scroll_panel, "Benchmarks", scroll_panel_content, &scroll_panel_scroll_offset, &scroll_panel_view);
            GuiSetStyle(DEFAULT, TEXT_SIZE, 32);
            BeginScissorMode(scroll_panel_view.x, scroll_panel_view.y, scroll_panel_view.width, scroll_panel_view.height);
            scroll_panel_content.height = 0;
            for(u32 i = 0; i < benchmark_list_count; i++) {
                if(benchmarks[i].name[0] != '\0') {
                    benchmark_tostr_ms(&benchmarks[i], benchmark_str_buffer);
                    GuiLabel((Rectangle){scroll_panel_view.x, (scroll_panel_view.y + 24 * (i+1)) + scroll_panel_scroll_offset.y, scroll_panel_view.width, 24}, benchmark_str_buffer);
                    scroll_panel_content.height += 26;
                } else break;
            }  
            EndScissorMode();
            GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
        }
        { // Draw bottom panel
            GuiDummyRec(bottom_bar_panel, NULL);
        }

        EndDrawing();
    }

    if(connected()) close_connection();
    CloseWindow();
    return (0);
}

void set_benchmarks_names_to_zero(void) {
    for(u32 i = 0; i < benchmark_list_count; i++) {
        memset(benchmarks[i].name, '\0', BENCHMARK_MAX_STRING_LENGTH);
    }
}

void device_version_callback(s32 major, s32 minor, s32 patch) {
    device_version[0] = major;
    device_version[1] = minor;
    device_version[2] = patch;
}

void benchmark_callback(benchmark_t benchmark) {
    u32 dif = benchmark.stop - benchmark.start;

    printf("Received benchmark with:\n start time: %d\n stop time: %d\n exec time: %d\n name: %s\n", 
    benchmark.start, benchmark.stop, dif, benchmark.name);
    for(u32 i = 0; i < benchmark_list_count; i++) {
        if(benchmarks[i].name[0] == '\0') {
            benchmarks[i].start = benchmark.start;
            benchmarks[i].stop = benchmark.stop;
            strncpy(benchmarks[i].name, benchmark.name, BENCHMARK_MAX_STRING_LENGTH);
            break;
        }
        if(strncmp(benchmarks[i].name, benchmark.name, BENCHMARK_MAX_STRING_LENGTH) == 0) {
            benchmarks[i].start = benchmark.start;
            benchmarks[i].stop = benchmark.stop;
            break;
        }
    }
}

void image_callback(image_t* image) {
    UnloadTexture(texture);
    Image image_local;
    image_local.width = image->cols;
    image_local.height = image->rows;
    image_local.format = PIXELFORMAT_UNCOMPRESSED_R5G6B5;
    image_local.mipmaps = 1;
    image_local.data = image->data;
    texture = LoadTextureFromImage(image_local);
}
