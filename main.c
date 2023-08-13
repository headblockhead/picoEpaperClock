#include "DEV_Config.h"
#include "Debug.h"
#include "EPD_2in13bc.h"
#include "GUI_Paint.h"
#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>
#include <stdlib.h> // malloc() free()

char *concat(const char *s1, const char *s2) {
  const size_t len1 = strlen(s1);
  const size_t len2 = strlen(s2);
  char *result = malloc(len1 + len2 + 1); // +1 for the null-terminator
  // in real code you would check for errors in malloc here
  memcpy(result, s1, len1);
  memcpy(result + len1, s2, len2 + 1); // +1 to copy the null-terminator
  return result;
}

char *wifiNetwork = "";
int wifiFound = 0;

static int scan_result(void *env, const cyw43_ev_scan_result_t *result) {
  if (result && !wifiFound) {
    wifiNetwork = concat(wifiNetwork, ",");
    wifiNetwork = concat(wifiNetwork, result->ssid);
    wifiFound = 1;
    //    printf("ssid: %-32s rssi: %4d chan: %3d mac:
    //    %02x:%02x:%02x:%02x:%02x:%02x "
    //           "sec: %u\n",
    //           result->ssid, result->rssi, result->channel, result->bssid[0],
    //           result->bssid[1], result->bssid[2], result->bssid[3],
    //           result->bssid[4], result->bssid[5], result->auth_mode);
  }
  return 0;
}

int main(void) {
  DEV_Delay_ms(500);

  // Wifi stuff
  stdio_init_all();

  if (cyw43_arch_init()) {
    printf("failed to initialise\n");
    return 1;
  }

  cyw43_arch_enable_sta_mode();

  absolute_time_t scan_time = nil_time;
  bool scan_in_progress = false;

  if (absolute_time_diff_us(get_absolute_time(), scan_time) < 0) {
    if (!scan_in_progress) {
      cyw43_wifi_scan_options_t scan_options = {0};
      int err = cyw43_wifi_scan(&cyw43_state, &scan_options, NULL, scan_result);
      if (err == 0) {
        printf("\nPerforming wifi scan\n");
        scan_in_progress = true;
      } else {
        printf("Failed to start scan: %d\n", err);
        scan_time = make_timeout_time_ms(10000); // wait 10s and scan again
      }
    } else if (!cyw43_wifi_scan_active(&cyw43_state)) {
      scan_time = make_timeout_time_ms(10000); // wait 10s and scan again
      scan_in_progress = false;
    }
  }
  // the following #ifdef is only here so this same example can be used in
  // multiple modes; you do not need it in your code
#if PICO_CYW43_ARCH_POLL
  // if you are using pico_cyw43_arch_poll, then you must poll periodically from
  // your main loop (not from a timer) to check for Wi-Fi driver or lwIP work
  // that needs to be done.
  cyw43_arch_poll();
  // you can poll as often as you like, however if you have nothing else to do
  // you can choose to sleep until either a specified time, or cyw43_arch_poll()
  // has work to do:
  cyw43_arch_wait_for_work_until(scan_time);
#else
  // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
  // is done via interrupt in the background. This sleep is just an example of
  // some (blocking) work you might be doing.
  sleep_ms(1000);
#endif
  sleep_ms(10000);

  // Initialize the GPIOs
  if (DEV_Module_Init() != 0) {
    return -1;
  }
  // Initialize the display.
  EPD_2IN13BC_Init();

  // Clear the display.
  EPD_2IN13BC_Clear();

  // The black colour and red colour are stored seperatly.

  // Create two images for black and red.
  UBYTE *BlackAndWhiteImage, *RedAndWhiteImage;
  UWORD Imagesize =
      ((EPD_2IN13BC_WIDTH % 8 == 0) ? (EPD_2IN13BC_WIDTH / 8)
                                    : (EPD_2IN13BC_WIDTH / 8 + 1)) *
      EPD_2IN13BC_HEIGHT;
  // Allocate memory for them.
  if ((BlackAndWhiteImage = (UBYTE *)malloc(Imagesize)) == NULL) {
    printf("Allocating memory for BlackAndWhiteImage failed.\r\n");
    return -1;
  }
  if ((RedAndWhiteImage = (UBYTE *)malloc(Imagesize)) == NULL) {
    printf("Allocating memory for RedAndWhiteImage failed.\r\n");
    return -1;
  }
  // Fill with white.
  Paint_NewImage(BlackAndWhiteImage, EPD_2IN13BC_WIDTH, EPD_2IN13BC_HEIGHT, 270,
                 WHITE);
  Paint_NewImage(RedAndWhiteImage, EPD_2IN13BC_WIDTH, EPD_2IN13BC_HEIGHT, 270,
                 WHITE);

  Paint_SelectImage(BlackAndWhiteImage);

  // Clear BlackAndWhite with white.
  Paint_SelectImage(BlackAndWhiteImage);
  Paint_Clear(WHITE);
  // Draw the black image.
  Paint_DrawString_EN(0, 15, "12345", &Font12, WHITE, BLACK);

  // Clear RedAndWhite with white.
  Paint_SelectImage(RedAndWhiteImage);
  Paint_Clear(WHITE);
  // Draw the red image.
  Paint_DrawString_EN(0, 0, "ABCD", &Font12, WHITE, BLACK);
  Paint_DrawLine(0, 12, EPD_2IN13BC_HEIGHT, 12, RED, DOT_PIXEL_1X1,
                 LINE_STYLE_DOTTED);
  Paint_DrawString_EN(0, 27, wifiNetwork, &Font12, WHITE, BLACK);

  // Display the red and black images as one image.
  EPD_2IN13BC_Display(BlackAndWhiteImage, RedAndWhiteImage);

  // Wait for 10 seconds to show the image.
  DEV_Delay_ms(10000);

  // Clear the screen to prevent burn-in during storage.
  EPD_2IN13BC_Clear();

  // Put the screen to sleep, uses minimal power.
  EPD_2IN13BC_Sleep();

  // Free the memory allocated to the images.
  free(BlackAndWhiteImage);
  free(RedAndWhiteImage);
  // Delete the images.
  BlackAndWhiteImage = NULL;
  RedAndWhiteImage = NULL;

  // Wait for the display to shut down - important! Must be at least two
  // seconds.
  DEV_Delay_ms(2000);

  // Stop sending 5v to the screen, turns it off, using no power.
  DEV_Module_Exit();

  // wifi
  cyw43_arch_deinit();

  return 0;
}
