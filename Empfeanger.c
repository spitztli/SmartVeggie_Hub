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
int weatherStationAdress = 0; // musss noch angepasst werden
int receivedAddress;
int bodenfeuchtigkeit;
int temperatur;
int luftfeuchtigkeit;
int msgCounterGrowStation = 0;
int msgCounterWeatherStation = 0;

int measurementData[2][3];
char* measurementDataName[2][3] = {{"Bodenf.: ", "Temperatur :", "Luftf. :"}, 
                                  {"Temperatur :", "Luftf. :", "Battery :"}}; //kontrollieren zuhause!!!!!!!!!!!!!!!
char* measurementValue[3] = {"%", " C", "%"};


LiquidCrystal_I2C lcd(0x27, 20, 4); 
const byte interruptPin = 3; 
const byte interruptLcdInitPin = 18;
int currentPage = 0;
int totalPages = 2;  
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 10;  // Entprellzeit in Millisekunden (Button)
bool buttonPressed = false;
volatile bool resetDisplayFlag = false;


ThreadController controller = ThreadController();
Thread* loraThread  = new Thread();
Thread* updateDisplayThread = new Thread();

void setup() {
  measurementData[1][0] = 1;
  measurementData[1][1] = 2;
  measurementData[1][2] = 3;
  Serial.begin(9600);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), navigationButton, CHANGE);
  pinMode(interruptLcdInitPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptLcdInitPin), setDisplayResetFlag, CHANGE);

  lcd.init();
  lcd.backlight();

  // Initialisierung von LoRa nur einmal
  
  Serial.println("LoRa Receiver");
  initializeLoRa();

  loraThread ->onRun(checkLoRa);
  loraThread ->setInterval(250);

  updateDisplayThread ->onRun(updateDisplay);
  updateDisplayThread ->setInterval(5000); 


  //controller.add(displayThread);
  controller.add(loraThread);
  controller.add(updateDisplayThread);
}

void loop() {
  controller.run();
  if (resetDisplayFlag) {
    lcd.init();
    lcd.backlight();
    Serial.println("LCD Init im Haupt-Loop");
    resetDisplayFlag = false; // Flag zurücksetzen
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
    if (LoRa.available() >= 4) {  //<--------auf 3 runter setzten
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
        Serial.print(" %, Battery: ");
        Serial.print(measurementData[1][2]);
        Serial.println("%");
      }
      else {
        Serial.println("Falsche Adresse!!!");
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
    lcd.print(measurementDataName[currentPage][i]); //kontrollieren zuhause!!!!!!!!!!!!!!!
    lcd.print(measurementData[currentPage][i]);
    lcd.print(measurementValue[i]); 
    Serial.println(measurementData[0][i]);
    Serial.println(measurementValue[i]);
  } 

  lcd.setCursor(0, 3);
  lcd.print("Counter Msg: ");
  lcd.print(msgCounterGrowStation);
}

void navigationButton() {
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTime) > debounceDelay) {
    int buttonState = digitalRead(interruptPin);
    if (buttonState == LOW && !buttonPressed) {
      // Button wurde gedrückt und vorher nicht gedrückt
      currentPage = (currentPage + 1) % totalPages;
      Serial.print("Interrupt ausgelöst: ");
      Serial.println(currentPage);
      buttonPressed = true;
    } else if (buttonState == HIGH && buttonPressed) {
      // Button wurde losgelassen
      buttonPressed = false;
    }
    lastDebounceTime = currentTime;
  }
}

void setDisplayResetFlag() {
  unsigned long currentTime = millis();
  if ((currentTime - lastDebounceTime) > debounceDelay) {
    resetDisplayFlag = true; // Setze das Flag im Interrupt-Handler
    lastDebounceTime = currentTime;
  }
}