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

char* messDaten[][3];


//wird zur Zeit nicht benötigt(Bewegungsmelder)
const int sensor = 2; // Pin für den Sensor
int val; 

LiquidCrystal_I2C lcd(0x27, 20, 4); 
const int buttonPin = 6;  //Button für Display navigiation 


ThreadController controller = ThreadController();
//Thread* displayThread = new Thread();
Thread* loraThread  = new Thread();
Thread* updateDisplayThread = new Thread();


unsigned long previousMillis = 0; 
const long interval = 300000; 

void setup() {
  pinMode(sensor, INPUT);
  Serial.begin(9600);

  Serial.println("LoRa Receiver");

  lcd.init();
  lcd.backlight();

  //displayThread->onRun(startDisplayLight);
  //displayThread->setInterval(100);

  // Initialisierung von LoRa nur einmal
  initializeLoRa();

  loraThread ->onRun(checkLoRa);
  loraThread ->setInterval(500);

  updateDisplayThread ->onRun(updateDisplay);
  updateDisplayThread ->setInterval(10000); // 10 Sekunden 


  //controller.add(displayThread);
  controller.add(loraThread);
  controller.add(updateDisplayThread);
}

void loop() {
  controller.run();
}


void startDisplayLight() {
  unsigned long currentMillis = millis();
  val = digitalRead(sensor);

  if (val) {
    Serial.println("Display Beleuchtung an!");
    lcd.backlight();
    previousMillis = currentMillis; // Reset Time
  }

  if (currentMillis - previousMillis >= interval && !val) {
    Serial.println("Display Beleuchtung ausgeschaltet!!");
    lcd.noBacklight(); 
    previousMillis = currentMillis; // Reset Timer
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
      receivedAddress = LoRa.read();
      messDaten[0][0] = LoRa.read();
      messDaten[0][1] = LoRa.read();
      messDaten[0][2] = LoRa.read();
      msgCounterLoraFlower++;
      //Serial.println(receivedAddress);

      if (receivedAddress == localAdress) {
        Serial.print("Bodenfeuchtigkeit: ");
        Serial.print(bodenfeuchtigkeit);
        Serial.print("%, Temp: ");
        Serial.print(temperatur);
        Serial.print(" C, Luftfeuchtigkeit: ");
        Serial.print(luftfeuchtigkeit);
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
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bodenf.: ");
  lcd.print(bodenfeuchtigkeit);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temperatur);
  lcd.print(" C");
  lcd.setCursor(0, 2);
  lcd.print("Feuchtigk.: ");
  lcd.print(luftfeuchtigkeit);
  lcd.print("%");
  lcd.setCursor(0, 3);
  lcd.print(" : ");
  lcd.print(msgCounterLoraFlower);
}

void navigationButton(int button) {
  int reading = button;
  if (reading == HIGH) {
    Serial.println("test");
    currentPage = (currentPage + 1) % totalPages;
    displayPage(currentPage);
  }
  lastButtonState = reading;
  delay(1000);
}


void displayPage(int page) {
  Serial.println(3);
  lcd.clear(); 

  for (int i = 0; i <= 3; i++) { 
    lcd.setCursor(0, i); 
    lcd.print(pages[page][i]); 
    Serial.println(pages[page][i]); 
  }    
}