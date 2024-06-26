#include "DHT.h"
#include <SPI.h>
#include <LoRa.h>

// Temperatur und Feuchtigkeit 
#define DHTPIN 3     
#define DHTTYPE DHT22   

DHT dht(DHTPIN, DHTTYPE);

// LoRa
const int csPin = 10;
const int resetPin = 9;
const int irqPin = 2;


int localAddress = 6258;


void setup() {
  Serial.begin(9600);
  initializeLoRa();
  dht.begin();
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Serial.print("Gelesene Temperatur: ");
  Serial.print(t);
  Serial.println("°C");
  Serial.print("Gelesene Luftfeuchtigkeit: ");
  Serial.print(h);
  Serial.println("%");

  sendMessage();
  delay(1000);
}


void sendMessage() {
  LoRa.beginPacket();
  LoRa.write((int)localAddress);
  LoRa.write((int)dht.readHumidity());    
  LoRa.write((int)dht.readTemperature()); 
  LoRa.write((char)"hallo");
  LoRa.endPacket();                       
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
