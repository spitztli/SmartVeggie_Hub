#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int csPin = 10;
const int resetPin = 9;
int growStationAdress = 38;
int weatherStationAdress = 114; // musss noch angepasst werden
int receivedAddress;
int bodenfeuchtigkeit;
int temperatur;
int luftfeuchtigkeit;
int msgCounterGrowStation = 0;
int msgCounterWeatherStation = 0;

int measurementData[2][3];
char* measurementDataName[2][3] = {{"Bodenf.: ", "Temperatur :", "Luftf. :"}, 
                                  {"Temperatur :", "Luftf. :", "Battery :"}}; //kontrollieren zuhause!!!!!!!!!!!!!!!
char* measurementValue[2][3] = {{"%", " C", "%"}, {"C", "%", "%"}};

LiquidCrystal_I2C lcd(0x27, 20, 4); 
const byte interruptPin = 3; 
const byte interruptLcdInitPin = 18;
int currentPage = 0;
int totalPages = 2;  
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;  // Erhöhen Sie die Entprellzeit auf 50 Millisekunden
volatile bool buttonPressed = false;
volatile bool resetDisplayFlag = false;

ThreadController controller = ThreadController();
Thread* periodicTaskThread = new Thread();

unsigned long lastTaskTime = 0;
bool taskRunning = false;
const unsigned long taskInterval = 60000; // 15 minutes in milliseconds
const unsigned long taskDuration = 30000; // 5 minutes in milliseconds

void setup() {
  Serial.begin(9600);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), navigationButtonISR, CHANGE); // Verwenden Sie FALLING, um Rauschen zu vermeiden
  pinMode(interruptLcdInitPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptLcdInitPin), setDisplayResetFlagISR, FALLING); // Verwenden Sie FALLING, um Rauschen zu vermeiden

  lcd.init();
  lcd.backlight();

  // Initialisierung von LoRa nur einmal
  Serial.println("LoRa Receiver");
  initializeLoRa();

  periodicTaskThread->onRun(handlePeriodicTask);
  periodicTaskThread->setInterval(250); // Check every second

  controller.add(periodicTaskThread);
}

void loop() {
  controller.run();
  if (resetDisplayFlag) {
    lcd.init();
    lcd.backlight();
    Serial.println("LCD Init im Haupt-Loop");
    resetDisplayFlag = false;
    updateDisplay(); // Flag zurücksetzen
  }

  // Check if button was pressed
  if (buttonPressed) {
    buttonPressed = false; // Reset the flag
    handleNavigationButton(); // Handle button press outside the ISR
    updateDisplay(); // Immediately update the display
  }
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

void checkLoRa() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Lese die Daten aus dem LoRa-Paket
    if (LoRa.available() >= 4) {  
      //Serial.print(LoRa.read());
      receivedAddress = LoRa.read();
      if (receivedAddress == growStationAdress) {
        measurementData[0][0] = LoRa.read();
        measurementData[0][1] = LoRa.read();
        measurementData[0][2] = LoRa.read();
        msgCounterGrowStation++;
        //Serial.println(receivedAddress);
        Serial.print("Bodenfeuchtigkeit: ");
        Serial.print(measurementData[0][0]);
        Serial.print("%, Temp: ");
        Serial.print(measurementData[0][1]);
        Serial.print(" C, Luftfeuchtigkeit: ");
        Serial.print(measurementData[0][2]);
        Serial.println("%");
      }
      if (receivedAddress == weatherStationAdress) {
        measurementData[1][0] = LoRa.read();
        measurementData[1][1] = LoRa.read();
        measurementData[1][2] = LoRa.read();
        msgCounterWeatherStation++;
        Serial.print("Temp: ");
        Serial.print(measurementData[1][0]);
        Serial.print("C, Luftfeuchtigkeit: ");
        Serial.print(measurementData[1][1]);
        Serial.print("%, Battery: ");
        Serial.print(measurementData[1][2]);
        Serial.println("%");
      }
    }
    else {
        Serial.println("Nicht genügend Daten verfügbar");
    }
  }
  else {
    Serial.println("Keine Daten angekommen!");
  }
}

void updateDisplay() {
  Serial.println(currentPage);
  lcd.clear();
  for (int i = 0; i < 3; i++) { 
    lcd.setCursor(0, i);
    lcd.print(measurementDataName[currentPage][i]);
    lcd.print(measurementData[currentPage][i]);
    lcd.print(measurementValue[currentPage][i]); 

  } 

  lcd.setCursor(0, 3);
  lcd.print("Counter Msg: ");
  lcd.print(msgCounterGrowStation);
}

void navigationButtonISR() {
  // Nur das Minimum im ISR ausführen
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
  // Nur das Minimum im ISR ausführen
  resetDisplayFlag = true;
}

void handlePeriodicTask() {
  unsigned long currentMillis = millis();

  if (taskRunning) {
    // Check if 5 minutes have passed since the task started
    if (currentMillis - lastTaskTime >= taskDuration) {
      // Stop the task
      taskRunning = false;
      Serial.println("Task stopped");
    }
  } else {
    // Check if 15 minutes have passed since the last task started
    if (currentMillis - lastTaskTime >= taskInterval) {
      // Start the task
      taskRunning = true;
      lastTaskTime = currentMillis;
      Serial.println("Task started");
    }
  }

  if (taskRunning) {
    checkLoRa();
  }
}
