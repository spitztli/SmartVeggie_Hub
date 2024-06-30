#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>
#include <dht.h>
#include <SPI.h>
#include <LoRa.h>

// Temperatur und Feuchtigkeit
dht DHT;
#define DHT11_PIN 4

// LoRa
const int csPin = 10;
const int resetPin = 9;
//const int irqPin = 2;

// Bodenfeuchtigkeit
const int sensorPin = A0;
const int dryValue = 512;   // Trockenheit
const int wetValue = 253;   // Feuchtigkeit

// Wasser Pumpe 
const int relayPinOne = 7;
const int relayPinTwo = 6; //Reserviert 
const int relayPinThree = 5; //Reserviert 
unsigned long pumpRunningTime = 0; // Laufzeit der Pumpe 
unsigned long lastMoistureCheckTime = 0;
const unsigned long moistureCheckInterval = 3600000; // 1 H

// Thread
ThreadController controller = ThreadController();
Thread* periodicTaskThread = new Thread();
Thread* pumpControlThread = new Thread();
Thread* loraSendThread = new Thread();

unsigned long lastTaskTime = 0;
bool taskRunning = false;
const unsigned long taskInterval = 900000; // 15 min pause 
const unsigned long taskDuration = 300000; // 5 min Sende Zeit 

// Arduino Adresse
int localAddress = 2342;

void setup() {
  pinMode(relayPinOne, OUTPUT);  
  pinMode(relayPinTwo, OUTPUT);  
  pinMode(relayPinThree, OUTPUT); 
  Serial.begin(9600);
  delay(2000); // 2 Sekunden Verzögerung zur Sensorstabilisierung
  initializeLoRa();
  
  periodicTaskThread->onRun(handlePeriodicTask);
  periodicTaskThread->setInterval(250); 

  pumpControlThread->onRun(steuereWasserpumpe);
  pumpControlThread->setInterval(10000); 

  loraSendThread->onRun(sendMessage);
  loraSendThread->setInterval(taskInterval); // 15 Sende Pause 

  controller.add(periodicTaskThread);
  controller.add(pumpControlThread);
  controller.add(loraSendThread);
}

void loop() {
  controller.run();
}

void sendMessage() {
  LoRa.beginPacket();
  LoRa.write((int)localAddress);
  LoRa.write(bodenfeuchtigkeitMessung()); 
  LoRa.write((int)DHT.temperature);      
  LoRa.write((int)DHT.humidity);          
  LoRa.endPacket();                
  Serial.println("Daten gesendet");
  Serial.print("Bodenfeuchtigkeit: ");
  Serial.print(bodenfeuchtigkeitMessung());
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


int bodenfeuchtigkeitMessung() {
  int sensorValue = analogRead(sensorPin);
  // Umrechnen des Sensorwertes in Prozent
  int prozent = map(sensorValue, wetValue, dryValue, 100, 0);
  // Begrenzen der Prozentwerte auf 0% bis 100%
  prozent = constrain(prozent, 0, 100);
  return prozent;
}

void steuereWasserpumpe() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastMoistureCheckTime >= moistureCheckInterval) {
    lastMoistureCheckTime = currentMillis; // Aktualisiere die Zeit der letzten Überprüfung

    int feuchtigkeit = bodenfeuchtigkeitMessung();
    Serial.print("Bodenfeuchtigkeit: ");
    Serial.println(feuchtigkeit);
    Serial.print("%");

    if (feuchtigkeit < 70) {
      Serial.println("Pumpe einschalten");
      digitalWrite(relayPinOne, HIGH);
      unsigned long startTime = millis(); // Startzeit speichern
      
      // Verwenden eines nicht-blockierenden Timers für die Pumpe
      while (millis() - startTime < 2000) {
        // Warte 2 Sekunden, während die Pumpe läuft (nicht blockierend)
      }
      
      digitalWrite(relayPinOne, LOW);
      Serial.println("Pumpe ausschalten"); 
      pumpRunningTime += (millis() - startTime); // Laufzeit hinzufügen

      unsigned long waitStartTime = millis();
      while (millis() - waitStartTime < 1800000) {
 
      }

      feuchtigkeit = bodenfeuchtigkeitMessung(); 
      if (feuchtigkeit < 70) {
        Serial.println("Pumpe erneut einschalten");
        digitalWrite(relayPinOne, HIGH); 
        startTime = millis();
        while (millis() - startTime < 2000) {
          // Warte 2 Sekunden, während die Pumpe läuft 
        }
        digitalWrite(relayPinOne, LOW);
        Serial.println("Pumpe ausschalten"); 
        pumpRunningTime += (millis() - startTime);
      }
    }
  }

  // Ausgabe der gesamten Pumplaufzeit
  Serial.print("Gesamte Pumplaufzeit: ");
  Serial.print(pumpRunningTime / 1000); // In Sekunden umrechnen
  Serial.println(" Sekunden");
}

void handlePeriodicTask() {
  unsigned long currentMillis = millis();

  if (taskRunning) {
     // Überprüfen, ob 5 Minuten seit dem Start der Aufgabe vergangen sind
    if (currentMillis - lastTaskTime >= taskDuration) {
      // Aufgabe stoppen
      taskRunning = false;
      Serial.println("Task stopped");
    }
  } else {
    // Überprüfen, ob 15 Minuten seit dem letzten Start der Aufgabe vergangen sind
    if (currentMillis - lastTaskTime >= taskInterval) {
      // Aufgabe starten
      taskRunning = true;
      lastTaskTime = currentMillis;
      Serial.println("Task started");
    }
  }

  if (taskRunning) {
    Serial.println("senden");
    sendMessage();
    Serial.println("senden");
  }
}

/*
void steuereWasserpumpe() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastMoistureCheckTime >= moistureCheckInterval) {
    lastMoistureCheckTime = currentMillis; // Aktualisiere die Zeit der letzten Überprüfung
    int feuchtigkeit = bodenfeuchtigkeitMessung();
    Serial.print("Bodenfeuchtigkeit: ");
    Serial.print(feuchtigkeit);
    Serial.println("%");
    
    while (feuchtigkeit < 70) {
      Serial.println("Pumpe einschalten");
      digitalWrite(relayPinOne, HIGH);
      delay(2000); // Pumpe 2 Sekunden laufen lassen
      digitalWrite(relayPinOne, LOW);
      Serial.println("Pumpe ausschalten");
      
      pumpRunningTime += 2000;
      
      delay(1800000); // 30 Minuten warten
      
      feuchtigkeit = bodenfeuchtigkeitMessung(); // Bodenfeuchtigkeit erneut messen
      Serial.print("Bodenfeuchtigkeit: ");
      Serial.print(feuchtigkeit);
      Serial.println("%");
    }
  }
}
*/
 