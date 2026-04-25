#include "secrets.h"
#include <WiFi.h>
#include "DHTesp.h"
#include <PubSubClient.h>

#define SENSOR_PIN 15    
#define RED_LED 13       
#define GREEN_LED 12     
#define BLUE_LED 27      

#define TEMP_THRESHOLD 25.0 
#define HUM_THRESHOLD 65.0  

const char* FEED_TEMP = "temperature";
const char* FEED_HUM  = "humidity";

#define MQTT_SERVER "io.adafruit.com"
#define MQTT_PORT 1883

DHTesp mySensor;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

unsigned long lastReconnectAttempt = 0;

boolean reconnect() {
  if (mqttClient.connect(AIO_USERNAME, AIO_USERNAME, AIO_KEY)) {
    Serial.println("MQTT Connected!");
  } else {
    Serial.print("MQTT failed, rc=");
    Serial.println(mqttClient.state());
  }
  return mqttClient.connected();
}

  void setup() {
  Serial.begin(115200);

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  mySensor.setup(SENSOR_PIN, DHTesp::DHT22);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
}


  void loop() {

  if (!mqttClient.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 3000) {
      lastReconnectAttempt = now;
      reconnect();
    }
  } else {
    mqttClient.loop();
  }

  TempAndHumidity data = mySensor.getTempAndHumidity();

  if (isnan(data.temperature) || isnan(data.humidity)) {
    Serial.println("Sensor error!");
    delay(2000);
    return;
  }

  float temperature = data.temperature;
  float humidity = data.humidity;

  bool tooHot = temperature > TEMP_THRESHOLD;
  bool tooHumid = humidity > HUM_THRESHOLD;

  if (mqttClient.connected()) {
    String topicTemp = String(AIO_USERNAME) + "/feeds/" + FEED_TEMP;
    String topicHum  = String(AIO_USERNAME) + "/feeds/" + FEED_HUM;

    mqttClient.publish(topicTemp.c_str(), String(temperature, 2).c_str());
    mqttClient.publish(topicHum.c_str(), String(humidity, 1).c_str());
  }

  if (tooHot && tooHumid) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, HIGH);

  } else if (tooHot) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, LOW);

  } else if (tooHumid) {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, HIGH);

  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, LOW);
  }

  Serial.println("---");
  Serial.println("Temperature: " + String(temperature, 2) + "°C");
  Serial.println("Humidity: " + String(humidity, 1) + "%");

  if (tooHot && tooHumid) {
    Serial.println("Status: HOT + HUMID");
  } else if (tooHot) {
    Serial.println("Status: HOT");
  } else if (tooHumid) {
    Serial.println("Status: HUMID");
  } else {
    Serial.println("Status: GOOD");
  }

  delay(2000);
}