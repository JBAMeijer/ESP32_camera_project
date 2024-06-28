//Server Code
#include <Arduino.h>
#include <esp_camera.h>
#include "config.h"
#include "benchmark.h"
#include "pc_interface.h"
#include "camera_pins.h"
#include "operators.h"
#include "mem_manager.h"

image_t* cam;

const char* ssid = "Camera_AP";
const char* password = "joeymeijer";

static const int test_bench_count = 6;

static benchmark_t benchmark[test_bench_count];
static bool test_bench_started[test_bench_count] = {false};
static int random_bench_time[test_bench_count];
static int bench_start_time[test_bench_count];


void setup() {
  delay(2000); // Give the terminal some time to connect to the device
  Serial.begin(115200);
  pc_wifi_interface_start(ssid, password);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);

  camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 18000000;
    config.frame_size = FRAMESIZE_QVGA;
    config.pixel_format = PIXFORMAT_RGB565;
    config.grab_mode = CAMERA_GRAB_LATEST;
    config.fb_location = CAMERA_FB_IN_DRAM;
    //config.jpeg_quality = 12;
    config.fb_count = 1;

    esp_err_t err = esp_camera_init(&config);
    if(err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

#if defined(USE_MEM_MANAGER)
    log_d("Used PSRAM: %d", ESP.getPsramSize() - ESP.getFreePsram());
    mem_manager_init();
    log_d("Used PSRAM: %d", ESP.getPsramSize() - ESP.getFreePsram());
#endif 
    cam = newRGB565Image(320, 240);
}

void loop() {
  delay(5000);
  pc_wifi_interface_update();
  camera_fb_t* camera_fb = esp_camera_fb_get();
  
  printf("Cam - w: %d, h: %d, len: %d\n", camera_fb->width, camera_fb->height, camera_fb->len);
  printf("First five bytes framebuffer: %d, %d, %d, %d, %d\n", 
    camera_fb->buf[0], camera_fb->buf[1], camera_fb->buf[2], camera_fb->buf[3], camera_fb->buf[4]);
  printf("First five bytes cam: %d, %d, %d, %d, %d\n", 
    cam->data[0], cam->data[1], cam->data[2], cam->data[3], cam->data[4]);
  benchmark_t b;
  
  benchmark_start(&b, "frame_copy");
  copy_framebuffer_to_rgb656Image(camera_fb, cam);
  benchmark_stop(&b);
  esp_camera_fb_return(camera_fb);

  printf("First five bytes cam again: %d, %d, %d, %d, %d\n", 
    cam->data[0], cam->data[1], cam->data[2], cam->data[3], cam->data[4]);

  char buf[50];
  benchmark_tostr(&b, buf);
  printf("%s", buf);

}