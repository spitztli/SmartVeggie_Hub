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
const int dryValue = 512;   // Trockenheit
const int wetValue = 253;   // Feuchtigkeit

int localAddress = 2342;
byte destination = 0x01;

void setup() {
  Serial.begin(9600);
  delay(2000); // 2 Sekunden Verz�gerung zur Sensorstabilisierung

 // Initialisierung von LoRa nur einmal
  initializeLoRa();
}

void loop() {
  
  int chk = DHT.read11(DHT11_PIN);

  sendMessage();
  delay(1000);
}

void sendMessage() {
  LoRa.beginPacket();
  LoRa.write((int)localAddress);
  LoRa.write(bodenfeuchtigkeitMessung()); // Diese Funktion sollte die Bodenfeuchtigkeit als Ganzzahl zurückgeben
  LoRa.write((int)DHT.temperature);       // Cast Temperatur zu int, vorausgesetzt, die DHT-Bibliothek gibt einen float zurück
  LoRa.write((int)DHT.humidity);          // Cast Feuchtigkeit zu int, gleiche Annahme wie Temperatur
  LoRa.endPacket();                       // Schließt das Paket ab und sendet es
  Serial.println("Daten gesendet");
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

int bodenfeuchtigkeitMessung() {
  int sensorValue = analogRead(sensorPin);
  // Umrechnen des Sensorwertes in Prozent
  int prozent = map(sensorValue, wetValue, dryValue, 100, 0);
  // Begrenzen der Prozentwerte auf 0% bis 100%
  prozent = constrain(prozent, 0, 100);
  Serial.println(prozent);
  return prozent;
}