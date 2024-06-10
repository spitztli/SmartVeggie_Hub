#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int csPin = 10;
const int resetPin = 9;

LiquidCrystal_I2C lcd(0x27, 20, 4); 


void setup() {
lcd.init();
lcd.backlight();
Serial.begin(9600);
while (!Serial);

Serial.println("LoRa Receiver");

LoRa.setPins(csPin, resetPin);
  
if (!LoRa.begin(433E6)) {
  Serial.println("Starting LoRa failed!");
  while (1);
  }

}

void loop() {
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