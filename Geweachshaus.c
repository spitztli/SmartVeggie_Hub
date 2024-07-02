#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>
#include "DHT.h"
#include <SPI.h>
#include <LoRa.h>

// Temperatur und Feuchtigkeit
#define DHTPIN 4     
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);

// LoRa
const int csPin = 10;
const int resetPin = 9;

// Bodenfeuchtigkeit
const int sensorPin = A0;
const int dryValue = 512;   // Trockenheit
const int wetValue = 253;   // Feuchtigkeit

// Wasser Pumpe 
const int relayPinOne = 7;

// Thread
ThreadController controller = ThreadController();
Thread* periodicTaskThread = new Thread();
Thread* pumpControlThread = new Thread();
Thread* loraSendThread = new Thread();

unsigned long lastTaskTime = 0;
bool taskRunning = false;
const unsigned long taskInterval = 20000; // 15 min pause 
const unsigned long taskDuration = 5000; // 5 min Sende Zeit 

// Arduino Adresse
int localAddress = 2342;

void setup() {
  pinMode(relayPinOne, OUTPUT);  
  Serial.begin(9600);
  delay(2000); // 2 Sekunden Verzögerung zur Sensorstabilisierung
  activateLoRa(); 
  dht.begin();
  
  periodicTaskThread->onRun(handlePeriodicTask);
  periodicTaskThread->setInterval(250); 

  pumpControlThread->onRun(watering);
  pumpControlThread->setInterval(5000); 

  loraSendThread->onRun(sendMessage);
  loraSendThread->setInterval(taskInterval); 

  controller.add(periodicTaskThread);
  controller.add(pumpControlThread);
  controller.add(loraSendThread);
}

void loop() {
  controller.run();
}

void sendMessage() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  for(int x = 0; x < 20; x++) {
    LoRa.beginPacket();
    LoRa.write((int)localAddress);
    LoRa.write(bodenfeuchtigkeitMessung()); 
    LoRa.write((int)t);      
    LoRa.write((int)h);          
    LoRa.endPacket();                
    Serial.println("Daten gesendet");
  }
}

void activateLoRa() {
    LoRa.setPins(csPin, resetPin);
    if (!LoRa.begin(433E6)) {
        Serial.println("LoRa activation failed!");
    } else {
        Serial.println("LoRa activated successfully!");
    }
}

void deactivateLoRa() {
    LoRa.end();  // Deaktiviere das LoRa-Modul
    Serial.println("LoRa deactivated");
}

int bodenfeuchtigkeitMessung() {
  int sensorValue = analogRead(sensorPin);
  // Umrechnen des Sensorwertes in Prozent
  int prozent = map(sensorValue, wetValue, dryValue, 100, 0);
  // Begrenzen der Prozentwerte auf 0% bis 100%
  prozent = constrain(prozent, 0, 100);
   
  return prozent;
}

void watering() {
  deactivateLoRa();
  int feuchtigkeit = bodenfeuchtigkeitMessung();
  Serial.print("Bodenfeuchtigkeit: ");
  Serial.print(feuchtigkeit);
  Serial.println("%");

  if (feuchtigkeit < 70) {
      Serial.println("Pumpe einschalten");
      digitalWrite(relayPinOne, HIGH);
      delay(2000);  
      digitalWrite(relayPinOne, LOW);
      Serial.println("Pumpe ausschalten");
      delay(5000); 

      feuchtigkeit = bodenfeuchtigkeitMessung(); 
      if (feuchtigkeit < 70) {
          Serial.println("Pumpe erneut einschalten");
          digitalWrite(relayPinOne, HIGH);
          delay(2000); 
          digitalWrite(relayPinOne, LOW);
          Serial.println("Pumpe ausschalten");
      }
  }
  delay(5000);
  activateLoRa(); //Aktiviert das LoRa-Modul
}

void handlePeriodicTask() {
    if (taskRunning) {
        sendMessage();  
        Serial.println("senden");
    }
}

