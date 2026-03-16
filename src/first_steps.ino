#define BUTTON_PIN 14
#define LED_PIN 4

int Buttonstate = LOW;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN,  OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  Buttonstate=   digitalRead(BUTTON_PIN);
  digitalWrite(LED_PIN, !Buttonstate);

  /*
  digitalWrite(LED_PIN, HIGH);
  delay(2000); // this speeds up the simulation
  digitalWrite(LED_PIN, LOW);
  delay(2000);
  */
}
