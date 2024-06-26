//Server Code
#define CORE_DEBUG_LEVEL 5
#include <Arduino.h>
#include "benchmark.h"
#include "pc_interface.h"

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
}

void loop() {

  delay(10);
  pc_wifi_interface_update();

  for(int i = 0; i < test_bench_count; i++)
  {
    if(!test_bench_started[i]) {
      char buf[32];
      snprintf(buf, 32, "test_bench_%d", i);
      benchmark_start(&benchmark[i], buf);
      random_bench_time[i] = (esp_random() % (10000 - 1000)) + 1000;
      bench_start_time[i] = millis();
      test_bench_started[i] = true;
    } else if(bench_start_time[i] + random_bench_time[i] < millis()) {
      benchmark_stop(&benchmark[i]);
      pc_wifi_interface_send_benchmark(&benchmark[i]);
      test_bench_started[i] = false;
    }
  }

  // if(!test_bench_started) {
  //   benchmark_start(&benchmark, "test_bench");
  //   random_bench_time = (esp_random() % (10000 - 1000)) + 1000;
  //   bench_start_time = millis();
  //   test_bench_started = true;
  // } else if(bench_start_time + random_bench_time < millis()) {
  //   benchmark_stop(&benchmark);
  //   pc_wifi_interface_send_benchmark(&benchmark);
  //   test_bench_started = false;
  // }

}