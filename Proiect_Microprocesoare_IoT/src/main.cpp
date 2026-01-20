#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>


const char* ssid = "DIGI_94e100"; 
const char* password = "aff56b1e";


const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;


const char* topic_obstacol = "proiect/garage/obstacol";
const char* topic_usa = "proiect/garage/usa";
const char* topic_temp = "proiect/garage/temperatura";


#define PIN_OBSTACOL 27  
#define PIN_USA 26       
#define PIN_TEMP 23      

WiFiClient espClient;
PubSubClient client(espClient);
OneWire oneWire(PIN_TEMP);
DallasTemperature sensors(&oneWire);

// Variabile Memorie
int lastObstacolState = -1;
int lastUsaState = -1;
unsigned long lastTempTime = 0;
unsigned long lastDebugTime = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("[WiFi] Conectare la ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] CONECTAT!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("[MQTT] Conectare...");
    String clientId = "ESP32_Test_" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Reusit!");
    } else {
      Serial.print("Esuat rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  
  pinMode(PIN_OBSTACOL, INPUT_PULLUP);
  pinMode(PIN_USA, INPUT_PULLUP);
  
  sensors.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int citireObstacol = digitalRead(PIN_OBSTACOL);
  int citireUsa = digitalRead(PIN_USA);

  
  if (millis() - lastDebugTime > 1000) {
    lastDebugTime = millis();
    Serial.print("Diagnostic -> Obstacol(P27): ");
    Serial.print(citireObstacol);
    Serial.print(" | Usa(P26): ");
    Serial.println(citireUsa);
  }

 
  if (citireObstacol != lastObstacolState) {
    
    if (citireObstacol == LOW) {
      Serial.println(">>> ALERTA: OBSTACOL (Semnal 0) <<<");
      client.publish(topic_obstacol, "BLOCAT");
    } else {
      Serial.println(">>> INFO: LIBER (Semnal 1) <<<");
      client.publish(topic_obstacol, "LIBER");
    }
    lastObstacolState = citireObstacol;
    delay(50);
  }

 
  if (citireUsa != lastUsaState) {
    if (citireUsa == LOW) {
      Serial.println(">>> USA: SENZOR ACTIV (Semnal 0) <<<");
      client.publish(topic_usa, "DESCHIS");
    } else {
      Serial.println(">>> USA: SENZOR INACTIV (Semnal 1) <<<");
      client.publish(topic_usa, "INCHIS");
    }
    lastUsaState = citireUsa;
    delay(50);
  }

  
  if (millis() - lastTempTime > 3000) {
    lastTempTime = millis();
    sensors.requestTemperatures();
    float t = sensors.getTempCByIndex(0);
    if (t > -100) {
      String tempStr = String(t, 1);
      client.publish(topic_temp, tempStr.c_str());
    }
  }
}