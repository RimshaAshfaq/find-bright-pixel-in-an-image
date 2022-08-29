#include "esp_camera.h"
#include <WiFi.h>

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM

#include "camera_pins.h"

const char* ssid = "robot";
const char* password = "12345678";

void startCameraServer();

void setup() {
 // pinMode (4, OUTPUT);
 // digitalWrite (4, HIGH);
  pinMode (33, OUTPUT);
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;         //5
  config.pin_d1 = Y3_GPIO_NUM;         //18
  config.pin_d2 = Y4_GPIO_NUM;         //19
  config.pin_d3 = Y5_GPIO_NUM;         //21
  config.pin_d4 = Y6_GPIO_NUM;         //36
  config.pin_d5 = Y7_GPIO_NUM;         //39
  config.pin_d6 = Y8_GPIO_NUM;         //34
  config.pin_d7 = Y9_GPIO_NUM;         //35
  config.pin_xclk = XCLK_GPIO_NUM;     //0
  config.pin_pclk = PCLK_GPIO_NUM;     //22
  config.pin_vsync = VSYNC_GPIO_NUM;   //25
  config.pin_href = HREF_GPIO_NUM;     //23
  config.pin_sscb_sda = SIOD_GPIO_NUM; //26
  config.pin_sscb_scl = SIOC_GPIO_NUM; //27
  config.pin_pwdn = PWDN_GPIO_NUM;     //32
  config.pin_reset = RESET_GPIO_NUM;   //-1
  config.xclk_freq_hz = 20000000;               
  config.pixel_format = PIXFORMAT_GRAYSCALE;
  //PIXFORMAT_YUV422
  //PIXFORMAT_GRAYSCALE
  //PIXFORMAT_RGB565
  //PIXFORMAT_JPEG
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; //1600*1200
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA; //800*600
    config.jpeg_quality = 12;
    config.fb_count = 1;
    // QVGA 320*240
    // CIF 400*296
    //VGA 640*480
    //XGA 1024*768
    // SXGA 1280*1024
  }
  

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }


  

  sensor_t * s = esp_camera_sensor_get();
                                                 // initial sensors are flipped vertically and colors are a bit saturated
   if (s->id.PID == OV3660_PID)
   {
    s->set_vflip(s, 1);                        // flip it back
    s->set_brightness(s, 1);                  // up the brightness just a bit
    s->set_saturation(s, -2);                // lower the saturation
//    s->special_effect(s, 0);                // 0 to 6
    //s->set_brightness(s, 0);     // -2 to 2
    s->set_contrast(s, 0);       // -2 to 2
    //s->set_saturation(s, 0);     // -2 to 2
    //s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
    s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
    s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
    s->set_aec2(s, 0);           // 0 = disable , 1 = enable
    s->set_ae_level(s, 0);       // -2 to 2
    s->set_aec_value(s, 300);    // 0 to 1200
    s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
    s->set_agc_gain(s, 0);       // 0 to 30
    s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
    s->set_bpc(s, 0);            // 0 = disable , 1 = enable
    s->set_wpc(s, 1);            // 0 = disable , 1 = enable
    s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
    s->set_lenc(s, 1);           // 0 = disable , 1 = enable
    s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
    //s->set_vflip(s, 0);          // 0 = disable , 1 = enable
    s->set_dcw(s, 1);            // 0 = disable , 1 = enable
    s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
  }




  
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  // put your main code here, to run repeatedly:
  //digitalWrite (33, HIGH);
  //delay(10000);
  //digitalWrite (33, LOW);
  //delay(10000);
}
