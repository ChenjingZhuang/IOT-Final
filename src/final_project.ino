#include "secrets.h"
#include <WiFi.h>
#include "DHTesp.h"
#include <PubSubClient.h>

#define SENSOR_PIN 15    
#define RED_LED 13       
#define GREEN_LED 12     
#define BLUE_LED 27      

#define TEMP_THRESHOLD 25.0 
#define HUM_THRESHOLD 70.0  

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
  bool tooHot = temperature > TEMP_THRESHOLD;
  bool tooHumid = humidity > HUM_THRESHOLD;
  // check FIRST

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Sensor error!");
    return;
  }

  // Publish to Adafruit IO
  String topicTemp = String(AIO_USERNAME) + "/feeds/" + FEED_TEMP;
  String topicHum  = String(AIO_USERNAME) + "/feeds/" + FEED_HUM;

if (mqttClient.connected()) {
  mqttClient.publish(topicTemp.c_str(), String(temperature, 2).c_str());
  mqttClient.publish(topicHum.c_str(), String(humidity, 1).c_str());
}
  if (tooHot && tooHumid) {
    // BOTH bad → PURPLE
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, HIGH);

  } else if (tooHot) {
    // Too hot → RED
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, LOW);

  } else if (tooHumid) {
    // Too humid → BLUE
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, HIGH);

  } else {
    // Good → GREEN
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
