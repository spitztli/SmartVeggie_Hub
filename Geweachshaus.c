#include <dht.h>
#include <SPI.h>
#include <LoRa.h>


dht DHT;
// Temperatur und Feuchtigkeit 
#define DHT11_PIN 4

// LoRa
const int csPin = 10;
const int resetPin = 9;
const int irqPin = 2;

//Bodenfeuchtigkeit
const int sensorPin = A0;
const int dryValue = 512;  // Wert f�r Trockenheit
const int wetValue = 253;  // Wert f�r hohe Feuchtigkeit

void setup() {
  Serial.begin(9600);
  delay(2000); // 2 Sekunden Verz�gerung zur Sensorstabilisierung

  while (!Serial);
  Serial.println("LoRa Sender");
  LoRa.setPins(csPin, resetPin, irqPin);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  int sensorValue = analogRead(sensorPin);
  // Umrechnen des Sensorwertes in Prozent
  int prozent = map(sensorValue, wetValue, dryValue, 100, 0);
  // Begrenzen der Prozentwerte auf 0% bis 100%
  prozent = constrain(prozent, 0, 100);
  
  int chk = DHT.read11(DHT11_PIN);


  // Ausgabe
  Serial.print("Bodenfeuchtigkeit: ");
  Serial.print(prozent);
  Serial.println("%");
  Serial.println("Temperature = " + String(DHT.temperature) + " Grad");
  Serial.println("Humidity = " + String(DHT.humidity) + "%");

  LoRa.beginPacket();
  LoRa.print("B-Feuchtigkeit: ");
  LoRa.print(prozent);
  LoRa.print(", Temp: ");
  LoRa.print(DHT.temperature);
  LoRa.print(", Hum: ");
  LoRa.print(DHT.humidity);
  LoRa.endPacket();
  
  delay(6000);
}