#include <WiFi.h>
#include <HTTPClient.h>
#include <cstdio>
#include "config.h"

#define MEAN_BUFF_SIZE 100

const char* ssid = SSID;
const char* password = WIFI_PASSWORD;
const char* serverName = SERVER_NAME;

unsigned long lastTime = 0;
unsigned long timerDelay = 300000;

const int sens_1_pin = 1;
const int sens_2_pin = 8;

void configure_adc(void);
void getNewVal(int);

typedef struct {
  int latestValue = 0;
  int meanBuffer[MEAN_BUFF_SIZE];
  int bufferIdx = 0;
  double mean = 0.0;
  bool hasWrapped = false;
  char name [32];
} sensor_t;

sensor_t sensor1 = {}; // mean room temp = ~2627, water = ~1225
sensor_t sensor2 = {}; // mean room temp = 2653, water = 1200

void setup() {

  strcpy(sensor1.name, "sensor1");
  strcpy(sensor2.name, "sensor2");

  Serial.begin(9600);

  if (password != nullptr && ssid != nullptr) {
    WiFi.begin(ssid, password);
  }

  Serial.println("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }
  Serial.println("");
  Serial.println("Connected to Wifi network with IP adress: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;

      http.begin(client, serverName);

      http.addHeader("Content-Type", "application/json");
      char httpRequestSensor1[128];
      snprintf(httpRequestSensor1, sizeof(httpRequestSensor1), "{\"sensorName\":\"%s\",\"humidity\":%f}", sensor1.name, sensor1.mean);

      char httpRequestSensor2[128];
      snprintf(httpRequestSensor2, sizeof(httpRequestSensor2), "{\"sensorName\":\"%s\",\"humidity\":%f}", sensor2.name, sensor2.mean);

      int httpResponseSensor1 = http.POST(httpRequestSensor1);
      int httpResponseSensor2 = http.POST(httpRequestSensor2);

      Serial.println("HTTP Response code: ");
      Serial.println(httpResponseSensor1);
      Serial.println(httpResponseSensor2);

      http.end();
    }
    else {
      Serial.println("Wifi Disconnected");
    }
    lastTime = millis();
  }

  sensor1.latestValue = analogRead(sens_1_pin);
  sensor2.latestValue = analogRead(sens_2_pin);

  getNewVal(&sensor1);
  getNewVal(&sensor2);
  
  if (sensor1.hasWrapped && sensor2.hasWrapped)
    Serial.printf("Sensor1: %f Sensor2: %f\n", sensor1.mean, sensor2.mean);
  delay(1000);
}

void configure_adc(void) {
  // TODO
}

void getNewVal(sensor_t *sens) {
  double sum = sens->mean * MEAN_BUFF_SIZE;
  sum -= sens->meanBuffer[sens->bufferIdx];
  sum += sens->latestValue;

  sens->mean = sum / MEAN_BUFF_SIZE;

  sens->meanBuffer[sens->bufferIdx] = sens->latestValue;

  ++sens->bufferIdx;

  if (sens->bufferIdx > 99) {
    sens->bufferIdx = 0;
    sens->hasWrapped = true;
  }
  
}

