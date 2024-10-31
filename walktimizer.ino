#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPSMinus.h>

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C // I2C address, either 0x3D or 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
TinyGPSMinus gps;
float legLength = 1.0;
int harmonic = 1;
float optimalSpeed;
float optimalPace;

// Define the button pin
const int downPin = 12;
const int upPin = 16;

enum State {
  PACE_DISPLAY,
  LEG_LENGTH
};

State currentState = PACE_DISPLAY;
unsigned long lastInputTime = 0;
const unsigned long inputTimeout = 3000;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial1.println("$PMTK251,9600*17");  // Set baud rate to 9600
  delay(1000);
  Serial1.println("$PMTK220,200*2C");  // Set update rate to 200ms
  delay(1000);

  // Configure the button pin
  pinMode(upPin, INPUT_PULLUP);  // Set the button pin as input with internal pull-up resistor
  pinMode(downPin, INPUT_PULLUP);  // Set the button pin as input with internal pull-up resistor
  pinMode(LED_BUILTIN, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { // Have a display showing what each field indicates (ex: Optimal Speed, Current Speed, Efficiency)
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  updateOptimal();
  delay(1000);
}

void updateState() {
  bool downPressed = digitalRead(downPin) == LOW;
  bool upPressed = digitalRead(upPin) == LOW;
  switch (currentState) {
    case PACE_DISPLAY:
      if (downPressed && upPressed) {
        currentState = LEG_LENGTH;
      }
      else if (upPressed && (harmonic <= 101)) {
        harmonic += 10;
        updateOptimal();
      } 
      else if (downPressed && (harmonic > 101)) {
        harmonic -= 10;
        updateOptimal();
      } 
      displayPace();
      break;

    case LEG_LENGTH:
      if (upPressed && downPressed) {
        currentState = PACE_DISPLAY;
      }
      else if (upPressed) {
        legLength += 0.1;
        updateOptimal();
      } 
      else if (downPressed) {
        legLength -= 0.1;
        updateOptimal();
      } 
      displayLegLength();
      break;
  }
}


void updateOptimal() {
  optimalSpeed = harmonic * sqrt(legLength * 9.81) / ((2 * 3.1415926535)*3.6);
  optimalPace = speedToPace(optimalSpeed);
}


float calculateEfficiency(float currentSpeed, float optimalSpeed) {
    float efficiency;
    efficiency = (1 - (fabs(optimalSpeed - currentSpeed) / optimalSpeed)) * 100.0;
    if (efficiency < 0){
      efficiency = 0;
    }
    return efficiency;
}


float speedToPace(float currentSpeed) {
  float currentPace = 60 / currentSpeed;
  return currentPace;
}


void formatPace(float pace, char* buffer) {
    int minutes = (int)pace;
    int seconds = (int)((pace - minutes) * 60);
    snprintf(buffer, 32, "%d'%02d\"", minutes, seconds);
}


void displayPace() {
  float currentSpeed = gps.f_speed_kmph();
  float currentPace = speedToPace(gps.f_speed_kmph());
  char buffer[32];

  // Calculate efficiency
  updateOptimal();
  float efficiency = calculateEfficiency(currentSpeed, optimalSpeed); 
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  if (currentPace > 0) {
        formatPace(optimalPace, buffer);
        display.print(buffer);
        display.println("/km");

        formatPace(currentPace, buffer);
        display.print(buffer);
        display.println("/km");

        // Display efficiency
        display.setTextSize(2.5);
        dtostrf(efficiency, -5, 2, buffer);
        display.print(buffer);
        display.print("%");
  } 
  else {
    display.setTextSize(2);
    display.println("Need...");
    display.println("Satellites");
    display.println("T__T");
  }
  display.display();
}

void displayLegLength() {
  char buffer[32];
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  formatPace(optimalPace, buffer);
  display.print(buffer);
  display.println("/km");

  dtostrf(legLength, 2, 1, buffer);
  display.print(buffer);
  display.println("m");
  
  display.display();
}

void loop() {
  while (Serial1.available()) {
    char c = Serial1.read();
    if (gps.encode(c)) {
      updateState();
    }
  }
}
