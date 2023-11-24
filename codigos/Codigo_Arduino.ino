#include "ArduinoJson.h"
#include "EspMQTTClient.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

// MQTT Configuracoes
EspMQTTClient client{
  "Wokwi-GUEST", //SSID do WiFi
  "",     // Senha do wifi
  "mqtt.tago.io",  // Endereço do servidor
  "Default",       // Usuario
  "7ab62891-fed9-4262-b561-e33237903ea1",         // Token do device
  "esp",           // Nome do device
  1883             // Porta de comunicação
};

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

float floatMap1(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float floatMap2(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int alerta_bpm;
int alerta_breathing;

void setup() {

  Serial.begin(9600);
  Serial.println("Conectando WiFi");
  while (!client.isWifiConnected()) {
    Serial.print('.'); client.loop(); delay(1000);
  }
  Serial.println("WiFi Conectado");
  Serial.println("Conectando com Servidor MQTT");
  while (!client.isMqttConnected()) {
    Serial.print('.'); client.loop(); delay(1000);
  }
  Serial.println("MQTT Conectado");

  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("failed to start SSD1306 OLED"));
    while (1);
  }
  oled.setCursor(0, 0);

  pinMode(34, INPUT);
  pinMode(32, INPUT);

}
// Callback da EspMQTTClient
void onConnectionEstablished()
{}
char bufferJson[100];

void loop() {
  
  oled.clearDisplay();
  oled.display();
  
  // read the input on analog pin GPIO36:
  int pote_bpm = analogRead(34);
  // Rescale to potentiometer's voltage (from 0V to 3.3V):
  float bpm = floatMap1(pote_bpm, 0, 4095, 60, 150);
  Serial.println(bpm);
 

  int pote_breathing = analogRead(32);
  // Rescale to potentiometer's voltage (from 0V to 3.3V):
  float breathing = floatMap2(pote_breathing, 0, 4095, 0, 10);
  Serial.println(breathing);


  if (bpm > 100) {
    alerta_bpm = 1;
  }
  else {
    alerta_bpm = 0;
  }

  if (breathing > 6) {
    alerta_breathing = 1;
  }
  else {
    alerta_breathing = 0;
  }

  if (alerta_bpm == 1){
    oled.setTextSize(1);         // set text size
    oled.setTextColor(WHITE);    // set text color
    oled.setCursor(0, 0);       // set position to display
    oled.println("Seus batimentos estao acelerados, pare de se movimentar e deixe o corpo relaxar"); // set text
    oled.display();
  }

  if (alerta_breathing == 1) {
    oled.setTextSize(1);         // set text size
    oled.setTextColor(WHITE);    // set text color
    oled.setCursor(0, 40);       // set position to display
    oled.println("Sua respiracao esta acelerada, pare, respire fundo e com calma"); // set text
    oled.display();
  }

    StaticJsonDocument<300> doc;
    doc["variable"] = "bpms";
    doc["value"] = bpm;
    serializeJson(doc, bufferJson);
    client.publish("topicAntiVicio", bufferJson);
    client.loop();
    delay(3000);

    doc["variable"] = "respiracao";
    doc["value"] = breathing;
    serializeJson(doc, bufferJson);
    client.publish("topicAntiVicio", bufferJson);
    client.loop();
    delay(3000);

}