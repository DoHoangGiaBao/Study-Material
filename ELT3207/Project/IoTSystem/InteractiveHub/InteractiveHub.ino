#define BLYNK_TEMPLATE_ID "TMPL6wLdaY8Lr"
#define BLYNK_TEMPLATE_NAME "Smart Home Security"
#define BLYNK_FIRMWARE_VERSION "0.0.3"
#define BLYNK_PRINT Serial
#define APP_DEBUG

#include <FS.h>
#include <SPIFFS.h>
using namespace fs;

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <time.h>
#include <esp_now.h>
#include "BlynkEdgent.h"

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600 * 7;
const int   daylightOffset_sec = 0; 

SPIClass mySpi = SPIClass(VSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

int batteryPercentage = 0;
int pirState = 0;
bool isArmed = false;
bool newDataReceived = false;
unsigned long lastMotionTime = 0;
const unsigned long MOTION_TIMEOUT = 5000;

typedef struct receivedMessage {
  int ps;
  int bp;
} receivedMessage;

receivedMessage pirData;

void drawDynamic();

BLYNK_WRITE(V1) {
  int value = param.asInt();
  isArmed = (value == 1);
  drawDynamic();
}

void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  memcpy(&pirData, incomingData, sizeof(pirData));
  batteryPercentage = pirData.bp;
  pirState = pirData.ps;
  newDataReceived = true;
}

void drawStatic() {
  tft.setTextColor(TFT_WHITE);
  tft.drawCentreString("Security Hub", TFT_HEIGHT / 2, 10, 4);
  tft.drawFastHLine(10, 40, TFT_HEIGHT - 20, TFT_LIGHTGREY);

  tft.setTextColor(TFT_WHITE);
  tft.drawString("Battery Capacity :", 10, 50, 4);
  tft.drawString("PIR : ", 10, 80, 4);
}

void drawDynamic() {
  tft.fillRect(TFT_HEIGHT / 2 + 60, 50, 80, 30, TFT_BLACK);

  if (batteryPercentage > 67) {
    tft.setTextColor(TFT_GREEN);
  } else if (batteryPercentage > 34) {
    tft.setTextColor(TFT_YELLOW);
  } else {
    tft.setTextColor(TFT_RED);
  }
  tft.drawString(String(batteryPercentage) + "%", TFT_HEIGHT / 2 + 60, 50, 4);

  Blynk.virtualWrite(V2, batteryPercentage);

  String message = "";
  tft.fillRect(70, 80, TFT_HEIGHT, 40, TFT_BLACK);
  if (isArmed) {
    tft.fillRoundRect(20, TFT_WIDTH / 2 + 10, TFT_HEIGHT - 40, 80, 10, TFT_RED);
    tft.setTextColor(TFT_WHITE);
    tft.drawCentreString("System Is Armed!", TFT_HEIGHT / 2, TFT_WIDTH / 2 + 20, 4);
    tft.drawCentreString("Click To Disarm", TFT_HEIGHT / 2, TFT_WIDTH / 2 + 60, 2);

    if (pirState == 0) {
      tft.setTextColor(TFT_GREEN);
      message = "No Motion Detected!";
      
    } else {
      tft.setTextColor(TFT_RED);
      message = "Motion Detected!";
    }
    tft.drawString(message, 70, 80, 4);

    struct tm timeInfo;
    if (!getLocalTime(&timeInfo, 10)) {
      message += "[No Date Data]";
    } else {
      char timeStringBuff[30];
      strftime(timeStringBuff, sizeof(timeStringBuff), " %Y-%m-%d %H:%M:%S", &timeInfo);
      message += String(timeStringBuff);
    }

    Blynk.virtualWrite(V0, message);
  } else {
    tft.fillRoundRect(20, TFT_WIDTH / 2 + 10, TFT_HEIGHT - 40, 80, 10, TFT_GREEN);
    tft.setTextColor(TFT_BLACK);
    tft.drawCentreString("System Is Disarmed!", TFT_HEIGHT / 2, TFT_WIDTH / 2 + 20, 4);
    tft.drawCentreString("Click To Arm", TFT_HEIGHT / 2, TFT_WIDTH / 2 + 60, 2);

    tft.setTextColor(TFT_CYAN);
    message = "No Message!";
    tft.drawString(message, 70, 80, 4);
  }
}

void checkTouch() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    
    int16_t x = map(p.x, 230, 3800, 0, TFT_HEIGHT);
    int16_t y = map(p.y, 340, 3900, 0, TFT_WIDTH);

    static unsigned long lastTouchTime = 0;
    if (millis() - lastTouchTime > 400) { 
      if (x > 20 && x < (TFT_HEIGHT - 20) && (y > TFT_WIDTH / 2 + 10) && y < (TFT_WIDTH / 2 + 10) + 80) {
        lastTouchTime = millis();
        isArmed = !isArmed;
        Blynk.virtualWrite(V1, isArmed ? 1 : 0);
        drawDynamic(); 
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);
  delay(100);

  mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(mySpi);
  ts.setRotation(1);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
  } else {
    esp_now_register_recv_cb(OnDataRecv);
    Serial.println("ESP-NOW Ready!");
  }

  drawStatic();
  drawDynamic();

  BlynkEdgent.begin();
}

void loop() {
  BlynkEdgent.run();
  checkTouch();

  if (newDataReceived) {
    newDataReceived = false;
    
    if (pirState == 1) {
      lastMotionTime = millis();
    }

    drawDynamic(); 
  }

  if (pirState == 1 && (millis() - lastMotionTime >= MOTION_TIMEOUT)) {
    pirState = 0;
    drawDynamic();
  }
}