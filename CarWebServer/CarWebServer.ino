#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiServer.h>
#include <WiFiClient.h>

#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"

// Wifi connection 
const char* ssid = 4 Chads";
const char* password = "";

WiFiServer server(4099);
WiFiClient client;
void startCameraServer();

// Pin setup
void setupLedFlash(int pin);
// Define motor control pins
const int motor1Pin1 = 14; // Connect to the IN1 pin on the L293D for motor 1
const int motor1Pin2 = 15; // Connect to the IN2 pin on the L293D for motor 1
const int motor2Pin1 = 13; // Connect to the IN3 pin on the L293D for motor 2
const int motor2Pin2 = 12; // Connect to the IN4 pin on the L293D for motor 2
// Enable pins
const int enableMotor1Pin = 2; // Connect to the IN3 pin on the L293D for motor 2
const int enableMotor2Pin = 4; // Connect to the IN4 pin on the L293D for motor 2

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

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
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(config.pixel_format == PIXFORMAT_JPEG){
    if(psramFound()){
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  if(config.pixel_format == PIXFORMAT_JPEG){
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

// Setup LED FLash if LED pin is defined in camera_pins.h
#if defined(LED_GPIO_NUM)
  setupLedFlash(LED_GPIO_NUM);
#endif

  // Server
  setUpPins();

  connectToWiFi();
  
  startCameraServer();
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

  server.begin();
  Serial.println("Server listening on port 4099");

  // Check for a client
  connectToPython();
}

int connection_counter = 0;

void loop() {
  // If a client is available, handle the communication
  if (client.connected())
  {
    if (!connection_counter) {
      Serial.println("Connected!");
      connection_counter++;
    }
    receiveDataFromServer();
  }
  else
  {
    disable();
    Serial.println("Client disconnected. Reconnecting...");
    connection_counter = 0;
    connectToPython();
  }
}

void setUpPins() {
  pinMode(motor1Pin1,OUTPUT);   //left motors  forward
  pinMode(motor1Pin2,OUTPUT);   //left motors reverse
  pinMode(motor2Pin1,OUTPUT);   //right  motors forward
  pinMode(motor2Pin2,OUTPUT);   //right motors reverse

  pinMode(enableMotor1Pin,OUTPUT);   //left motors enable
  pinMode(enableMotor2Pin,OUTPUT);   //right motors enable
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void connectToPython() {
  // Check for a client
  client = server.available();
  while (!client)
  {
    client = server.available();
    delay(500); // Wait for a second before checking again
  }
}

char c;
char old_c;

void receiveDataFromServer() {  
  c = client.read();
  if(!c)
  {
    c == old_c;
  }
  commandCar(c);
  old_c = c;
}

void commandCar(char command) {
  switch(command) {
    case('f'):
      enable();
      moveForward();
      break;

    case('b'):
      enable();
      moveBackward();
      break;

    case('l'):
      enable();
      turnLeft();
      break;

    case('r'):
      enable();
      turnRight();
      break;

    case('s'):
      disable();
      break;
  }
}

void turnRight() {
  Serial.println("right");
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
}

void turnLeft() {
  Serial.println("left");
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
}

void moveForward() {
  Serial.println("forward");
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
}

void moveBackward() {
  Serial.println("backward");
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
}

void enable() {
  digitalWrite(enableMotor1Pin, HIGH);
  digitalWrite(enableMotor2Pin, HIGH);
}

void disable() {
  digitalWrite(enableMotor1Pin, LOW);
  digitalWrite(enableMotor2Pin, LOW);
}
