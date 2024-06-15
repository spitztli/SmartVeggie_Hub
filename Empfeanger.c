#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>


#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int csPin = 10;
const int resetPin = 9;
int localAdress = 38;
int receivedAddress;
int bodenfeuchtigkeit;
int temperatur;
int luftfeuchtigkeit;
int msgCounterLoraFlower = 0;

int measurementData[2][3];
char* measurementDataName[] = {"Bodenf.: ", "Temperatur :", "Luftf. :"};
char* measurementValue[3] = {"%", " C", "%"};


LiquidCrystal_I2C lcd(0x27, 20, 4); 
const byte interruptPin = 3; 
int currentPage = 0;
int totalPages = 2;  


ThreadController controller = ThreadController();
Thread* loraThread  = new Thread();
Thread* updateDisplayThread = new Thread();

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 10;  // Entprellzeit in Millisekunden (Button)
bool buttonPressed = false;


void setup() {
  measurementData[1][0] = 1;
  measurementData[1][1] = 2;
  measurementData[1][2] = 3;
  Serial.begin(9600);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), navigationButton, CHANGE);

  //pinMode(sensor, INPUT);
  Serial.println("LoRa Receiver");

  lcd.init();
  lcd.backlight();

  // Initialisierung von LoRa nur einmal
  initializeLoRa();

  loraThread ->onRun(checkLoRa);
  loraThread ->setInterval(250);

  updateDisplayThread ->onRun(updateDisplay);
  updateDisplayThread ->setInterval(500); 


  //controller.add(displayThread);
  controller.add(loraThread);
  controller.add(updateDisplayThread);
}

void loop() {
  controller.run();
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
      receivedAddress = LoRa.read();
      measurementData[0][0] = LoRa.read();
      measurementData[0][1] = LoRa.read();
      measurementData[0][2] = LoRa.read();
      msgCounterLoraFlower++;
      //Serial.println(receivedAddress);

      if (receivedAddress == localAdress) {
        Serial.print("Bodenfeuchtigkeit: ");
        Serial.print(measurementData[0][0]);
        Serial.print("%, Temp: ");
        Serial.print(measurementData[0][1]);
        Serial.print(" C, Luftfeuchtigkeit: ");
        Serial.print(measurementData[0][2]);
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
    lcd.print(measurementDataName[i]); 
    lcd.print(measurementData[currentPage][i]);
    lcd.print(measurementValue[i]); 
    Serial.println(measurementData[0][i]);
    Serial.println(measurementValue[i]);
  } 

  lcd.setCursor(0, 3);
  lcd.print("Counter Msg: ");
  lcd.print(msgCounterLoraFlower);
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