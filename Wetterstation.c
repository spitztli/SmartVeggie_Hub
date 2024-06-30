#include <LowPower.h>
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
//const int irqPin = 2;

// Sendeintervall und Sendedauer in Millisekunden
const unsigned long taskInterval = 1800000; // 30 min Pause
const unsigned long taskDuration = 300000; // 5 min Sendezeit

// Arduino Adresse
int localAddress = 6258;

void setup() {
  Serial.begin(9600);
  initializeLoRa();
  dht.begin();
}

void loop() {
  // 5 Minuten lang Daten erfassen und senden
  unsigned long startMillis = millis();
  while (millis() - startMillis < taskDuration) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    Serial.print("Gelesene Temperatur: ");
    Serial.print((int)t); // Umwandlung in Ganzzahl für Anzeige
    Serial.println("°C");
    Serial.print("Gelesene Luftfeuchtigkeit: ");
    Serial.print((int)h); // Umwandlung in Ganzzahl für Anzeige
    Serial.println("%");
    sendMessage();
    delay(250); // Optional: Wartezeit zwischen den Nachrichten, um das Senden zu verlangsamen
  }

  // Mikrocontroller für 30 Minuten in den Schlafmodus versetzen
  for (int i = 0; i < (taskInterval / 8000); i++) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
}

void sendMessage() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  LoRa.beginPacket();
  LoRa.write((int)localAddress);
  LoRa.write((int)h);    
  LoRa.write((int)t); 
  LoRa.write((char*)"hallo"); // Diese Nachricht wird für die Batterie verwendet
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
