#include <WiFi.h>
#include "DHTesp.h"
#include <PubSubClient.h>

#define SENSOR_PIN 15    
#define RED_LED 13       
#define GREEN_LED 12     
#define BLUE_LED 27      

#define TEMP_THRESHOLD 25.0 
#define HUM_THRESHOLD 70.0  

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// Adafruit IO MQTT
const char* AIO_USERNAME = "chenj";
const char* AIO_KEY = "aio_eBMR04zT5tYqcWZivcdZUhYL00r8";
const char* FEED_TEMP = "temperature";
const char* FEED_HUM  = "humidity";

#define MQTT_SERVER "io.adafruit.com"
#define MQTT_PORT 1883

DHTesp mySensor;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

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
  Serial.print("Connecting to Adafruit IO");
  while (!mqttClient.connected()) {
    if (mqttClient.connect(AIO_USERNAME, AIO_USERNAME, AIO_KEY)) {
      Serial.println(" Connected!");
    } else {
      Serial.print(" failed, rc=");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }
}

void loop() {
  if (!mqttClient.connected()) {
    while (!mqttClient.connect(AIO_USERNAME, AIO_USERNAME, AIO_KEY)) {
      delay(2000);
    }
  }
  mqttClient.loop();

  TempAndHumidity data = mySensor.getTempAndHumidity();
  float temperature = data.temperature;
  float humidity = data.humidity;

  // Publish to Adafruit IO
  mqttClient.publish((String(AIO_USERNAME) + "/feeds/" + FEED_TEMP).c_str(), String(temperature, 2).c_str());
  mqttClient.publish((String(AIO_USERNAME) + "/feeds/" + FEED_HUM).c_str(), String(humidity, 1).c_str());

  if (temperature > TEMP_THRESHOLD || humidity > HUM_THRESHOLD) {
    // Bad environment → RED
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
  } else {
    // Good environment → GREEN
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, LOW);
  }


  Serial.println("---");
  Serial.println("Temperature: " + String(temperature, 2) + "°C");
  Serial.println("Humidity: " + String(humidity, 1) + "%");

  delay(2000); 