#include "FS.h"
#include "SD_MMC.h"
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPI.h"
#include <DFRobot_DHT20.h>
#include <Wire.h>
DFRobot_DHT20 dht20;

const char* ssid = "Grimbo";
const char* password = "sveveliko";

const int comPin1 = 1;
const int comPin2 = 3;

const int temperaturePin = 33;
const int humidityPin = 0;

float temperatureValue = 4.0;
float humidityValue = 2.0;

AsyncWebServer server(80);
void initMicroSDCard() {
  Serial.println("Mounting MicroSD Card");
  if (!SD_MMC.begin()) {
    Serial.println("MicroSD Card Mount Failed");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No MicroSD Card found");
    return;
  }

}
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void setup() {

  pinMode(humidityPin, OUTPUT);
  pinMode(temperaturePin, OUTPUT);

  Serial.begin(115200);
  Wire.begin(comPin1, comPin2);
  initMicroSDCard();
  initWiFi();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SD_MMC, "/index.html", "text/html");
  });
  server.on("/sensor-data", HTTP_GET, [](AsyncWebServerRequest *request) {
    String temperature = String(temperatureValue, 2);
    String humidity = String(humidityValue, 2);
    String json = "{\"temperature\": " + temperature + ", \"humidity\": " + humidity + "}";
    request->send(200, "application/json", json);
  });
  server.serveStatic("/", SD_MMC, "/");
  server.begin();

  TimerHandle_t timer = xTimerCreate("sensorTimer", pdMS_TO_TICKS(2000), pdTRUE, (void *)0, [](TimerHandle_t xTimer) {
    temperatureValue = dht20.getTemperature();
    humidityValue = dht20.getHumidity()*100;

    if (humidityValue >= 45)
    {
      digitalWrite(humidityPin, HIGH);
    }
    else
    {
      digitalWrite(humidityPin, LOW);
    }
    if (temperatureValue >= 40)
    {
      analogWrite(temperaturePin, 255);
    }
    else if (temperatureValue <= 0)
    {
      analogWrite(temperaturePin, 0);
    }
    else
    {
      int brightness = map(temperatureValue, 0, 40, 0, 255);
      analogWrite(temperaturePin, brightness);
    }

  });
  xTimerStart(timer, 0);
}
void loop() {
}