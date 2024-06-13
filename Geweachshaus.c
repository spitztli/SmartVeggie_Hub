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

byte localAddress = 0x02;     
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
  LoRa.write(bodenfeuchtigkeitMessung()); 
  LoRa.write((int)DHT.temperature);       
  LoRa.write((int)DHT.humidity);          
  LoRa.endPacket();                       // Schließt das Paket ab und sendet es
  Serial.println("Daten gesendet");

  /*              
  //LoRa.write(outgoing.length());        // add payload length
  //LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  //msgCount++;                           // increment message ID
  Serial.println("Sending successful!");
  Serial.println(DHT.temperature);
  */
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
}