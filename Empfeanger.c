#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Konstanten für die Pinbelegung und Adressen
#define CS_PIN 10
#define RESET_PIN 9
#define GROW_STATION_ADDRESS 38
#define NEW_STATION 114
#define INTERRUPT_PIN 3
#define INTERRUPT_LCD_INIT_PIN 18
#define LCD_ADDRESS 0x27

// Messdaten Arrays und Zähler
int measurementData[2][3];
const char* measurementDataName[2][3] = {
  {"Bodenf.: ", "Temperatur :", "Luftf. :"}, 
  {"Temperatur :", "Luftf. :", "Battery :"}
};
const char* measurementValue[2][3] = {{"%", " C", "%"}, {"C", "%", "%"}};
int msgCounterGrowStation = 0;
int msgCounterWeatherStation = 0;
int* counterList[2] = {&msgCounterGrowStation, &msgCounterWeatherStation};

LiquidCrystal_I2C lcd(LCD_ADDRESS, 20, 4); 

// Seitennavigation Variablen
volatile bool buttonPressed = false;
volatile bool resetDisplayFlag = false;
int currentPage = 0;
const int totalPages = 2;  
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200;  

// Periodische Aufgaben Variablen
ThreadController controller = ThreadController();
Thread* periodicTaskThread = new Thread();
unsigned long lastTaskTime = 0;
bool taskRunning = false;
const unsigned long taskInterval = 60000; // 15 min
const unsigned long taskDuration = 30000; // 5 min

void setup() {
  Serial.begin(9600);
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), navigationButtonISR, CHANGE); 
  pinMode(INTERRUPT_LCD_INIT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(INTERRUPT_LCD_INIT_PIN), setDisplayResetFlagISR, FALLING); 

  initializeLCD();
  initializeLoRa();

  periodicTaskThread->onRun(handlePeriodicTask);
  periodicTaskThread->setInterval(250); 
  controller.add(periodicTaskThread);
}

void loop() {
  controller.run();
  if (resetDisplayFlag) {
    initializeLCD();
    resetDisplayFlag = false;
    updateDisplay(); 
  }

  if (buttonPressed) {
    buttonPressed = false; 
    handleNavigationButton(); 
    updateDisplay(); 
  }
}

void initializeLCD() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void initializeLoRa() {
  LoRa.setPins(CS_PIN, RESET_PIN);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    return;
  }
  Serial.println("LoRa Initialized Successfully!");
}

void checkLoRa() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    if (LoRa.available() >= 4) {
      int receivedAddress = LoRa.read();
      if (receivedAddress == GROW_STATION_ADDRESS) {
        measurementData[0][0] = LoRa.read();
        measurementData[0][1] = LoRa.read();
        measurementData[0][2] = LoRa.read();
        (*counterList[0])++;
        Serial.print("Bodenfeuchtigkeit: ");
        Serial.print(measurementData[0][0]);
        Serial.print("%, Temp: ");
        Serial.print(measurementData[0][1]);
        Serial.print(" C, Luftfeuchtigkeit: ");
        Serial.print(measurementData[0][2]);
        Serial.println("%");
      } else if (receivedAddress == NEW_STATION) {
        measurementData[1][0] = LoRa.read();
        measurementData[1][1] = LoRa.read();
        measurementData[1][2] = LoRa.read();
        (*counterList[1])++;
        Serial.print("Temp: ");
        Serial.print(measurementData[1][0]);
        Serial.print("C, Luftfeuchtigkeit: ");
        Serial.print(measurementData[1][1]);
        Serial.print("%, Battery: ");
        Serial.print(measurementData[1][2]);
        Serial.println("%");
      }
    } else {
      Serial.println("Nicht genügend Daten verfügbar");
    }
  } else {
    Serial.println("Keine Daten angekommen!");
  }
}

void updateDisplay() {
  lcd.clear();
  for (int i = 0; i < 3; i++) { 
    lcd.setCursor(0, i);
    lcd.print(measurementDataName[currentPage][i]);
    lcd.print(measurementData[currentPage][i]);
    lcd.print(measurementValue[currentPage][i]); 
  } 
  lcd.setCursor(0, 3);
  lcd.print("Counter Msg: ");
  lcd.print(*counterList[currentPage]);
}

void navigationButtonISR() {
  buttonPressed = true;
}

void handleNavigationButton() {
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTime) > debounceDelay) {
    currentPage = (currentPage + 1) % totalPages;
    Serial.print("Interrupt ausgelöst: ");
    Serial.println(currentPage);
    lastDebounceTime = currentTime;
  }
}

void setDisplayResetFlagISR() {
  resetDisplayFlag = true;
}

void handlePeriodicTask() {
  unsigned long currentMillis = millis();

  if (taskRunning) {
    if (currentMillis - lastTaskTime >= taskDuration) {
      taskRunning = false;
      Serial.println("Task stopped");
    }
  } else {
    if (currentMillis - lastTaskTime >= taskInterval) {
      taskRunning = true;
      lastTaskTime = currentMillis;
      Serial.println("Task started");
    }
  }

  if (taskRunning) {
    checkLoRa();
  }
}
