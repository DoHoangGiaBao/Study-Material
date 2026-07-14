#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#define PIR_PIN 13
#define BATTERY_PIN 32

uint8_t hubAddress[] = {0xB4, 0xBF, 0xE9, 0x0E, 0x70, 0x40};

typedef struct sendedMessage {
  int pirState;
  int batteryPercentage;
} sendedMessage;

sendedMessage payload;
esp_now_peer_info_t peerInfo;

volatile bool replyReceived = false;
volatile bool deliverySuccess = false;

float readBattery() {
  int rawADC = analogRead(BATTERY_PIN);
  float batteryVoltage = (rawADC / 4095.0) * 3.3 * 2.0;
  float percentage = ((batteryVoltage - 3.0) / (5.0 - 3.0) * 100.0);
  
  if(percentage > 100.0) percentage = 100.0;
  if(percentage < 0.0) percentage = 0.0;
  
  return percentage;
}

void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  replyReceived = true;
  deliverySuccess = (status == ESP_NOW_SEND_SUCCESS);
  Serial.print("Delivery Status: ");
  Serial.println(deliverySuccess ? "SUCCESS (Hub Received It)" : "FAIL (Hub Missed It)");
}

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  payload.pirState = digitalRead(PIR_PIN);
  payload.batteryPercentage = readBattery();

  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  memcpy(peerInfo.peer_addr, hubAddress, 6);
  peerInfo.channel = 6;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  int retryAttempts = 0;
  const int maxRetries = 3;
  
  while(retryAttempts < maxRetries) {
    replyReceived = false;
    deliverySuccess = false;

    Serial.print("Sending packet, attempt: ");
    Serial.println(retryAttempts + 1);

    esp_err_t result = esp_now_send(hubAddress, (uint8_t *) &payload, sizeof(payload));
    
    if (result == ESP_OK) {
      unsigned long startWait = millis();
      while (!replyReceived && (millis() - startWait < 100)) {
        delay(1); 
      }
      
      if (deliverySuccess) {
        digitalWrite(2, HIGH);
        break; 
      }
    }

    retryAttempts++;
    delay(50);
  }

  while (digitalRead(PIR_PIN) == HIGH) {
    delay(10);
  }

  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIR_PIN, HIGH); 
  delay(100);
  esp_deep_sleep_start();

  Serial.println("Going to sleep now");
  digitalWrite(2, LOW);
 
}

void loop() {
  // Not used
}