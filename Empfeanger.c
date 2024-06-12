#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int csPin = 10;
const int resetPin = 9;
const int sensor = 2; // Pin für den Sensor
int val; 

LiquidCrystal_I2C lcd(0x27, 20, 4); 

ThreadController controller = ThreadController();
Thread* displayThread = new Thread();
Thread* loraThread  = new Thread();

unsigned long previousMillis = 0; 
const long interval = 3000; 

void setup() {
  pinMode(sensor, INPUT);
  Serial.begin(9600);

  Serial.println("LoRa Receiver");

  lcd.init();
  lcd.backlight();

  displayThread->onRun(startDisplayLight);
  displayThread->setInterval(100);

  // Initialisierung von LoRa nur einmal
  initializeLoRa();

  loraThread ->onRun(checkLoRa);
  loraThread ->setInterval(3000);

  controller.add(displayThread);
  controller.add(loraThread );
}

void loop() {
  controller.run();
}


void startDisplayLight() {
  unsigned long currentMillis = millis();
  val = digitalRead(sensor);

  if (val) {
    Serial.println("Display Beleuchtung an!");
    lcd.backlight();
    previousMillis = currentMillis; // Reset Time
  }

  if (currentMillis - previousMillis >= interval && !val) {
    Serial.println("Display Beleuchtung ausgeschaltet!!");
    lcd.noBacklight(); 
    previousMillis = currentMillis; // Reset Timer
  }
}


void initializeLoRa() {
  LoRa.setPins(csPin, resetPin);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    // Fehlerbehandlung, z.B. einen Zustand setzen, der angibt, dass LoRa nicht verfügbar ist
    return;
  }
  Serial.println("LoRa Initialized Successfully!");
}


void checkLoRa() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }

    Serial.println(received); // Ausgabe der empfangenen Daten

    // Trennen der Nachricht in die einzelnen Teile
    int firstCommaIndex = received.indexOf(',');
    int secondCommaIndex = received.indexOf(',', firstCommaIndex + 1);

    String prozent = received.substring(15, firstCommaIndex); 
    String tempStr = received.substring(firstCommaIndex + 6, secondCommaIndex); 
    String humStr = received.substring(secondCommaIndex + 5);

    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("B-Feuchtigkeit:");
    lcd.print(prozent);
    lcd.setCursor(0, 1);
    lcd.print("Temp:");
    lcd.print(tempStr);
    lcd.setCursor(0, 2);
    lcd.print("Hum:");
    lcd.print(humStr);
  }
}