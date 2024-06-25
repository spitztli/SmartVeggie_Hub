#include <dht.h>
#include <SPI.h>
#include <LoRa.h>

// Temperatur und Feuchtigkeit 
dht DHT;
#define DHT11_PIN 4

// LoRa
const int csPin = 10;
const int resetPin = 9;
const int irqPin = 2;

int localAddress = 6258;
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
  LoRa.write((int)DHT.temperature);       // Cast Temperatur zu int, vorausgesetzt, die DHT-Bibliothek gibt einen float zurück
  LoRa.write((int)DHT.humidity);          // Cast Feuchtigkeit zu int, gleiche Annahme wie Temperatur
  LoRa.endPacket();                       // Schließt das Paket ab und sendet es
  Serial.println("Daten gesendet");
  Serial.print("%, Temp: ");
  Serial.print(DHT.temperature);
  Serial.print(" C, Luftfeuchtigkeit: ");
  Serial.print(DHT.humidity);
  Serial.println("%");
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