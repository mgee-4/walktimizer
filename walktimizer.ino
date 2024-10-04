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
float legLength = 1;
int harmonic = 1;
float optimalSpeed;
float optimalPace;

// Define the button pin
const int upPin = 12;
const int downPin = 16;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial1.println("$PMTK251,9600*17");  // Set baud rate to 9600
  delay(1000);
  Serial1.println("$PMTK220,200*2C");  // Set update rate to 200ms
  delay(1000);

  // Configure the button pin
  pinMode(downPin, INPUT_PULLUP);  // Set the button pin as input with internal pull-up resistor
  pinMode(upPin, INPUT_PULLUP);  // Set the button pin as input with internal pull-up resistor
  pinMode(LED_BUILTIN, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { // Have a display showing what each field indicates (ex: Optimal Speed, Current Speed, Efficiency)
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  updateOptimal();  // Placeholder, need to implement it so that this is called when users adjust leg length manually
  delay(1000);
}


// Need to set up FSM between user input mode and speed checking/displaying, currently just printing speed/efficiency
void loop() {
  bool upPressed = digitalRead(upPin) == LOW;
  bool downPressed = digitalRead(downPin) == LOW;
  if (downPressed && upPressed){
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(1000);                      // wait for a second
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  }
  else if (upPressed) {
    displayUpButtonPressed();  // Display "UP" message when up button is pressed
  } 
  else if (downPressed) {
    displayDownButtonPressed();  // Display "DOWN" message when down button is pressed
  }
  else {
    while (Serial1.available()) {
      char c = Serial1.read();
      if (gps.encode(c)) {
        displayPace();  // Display pace if GPS data is available
      }
    }
  }
}

// Temporary function to test button inputs and debouncing
void displayRightButtonPressed() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Right");
  display.println("Pressed!");
  display.display();
  delay(100);  // Debounce delay
}

// Temporary function to test button inputs and debouncing
void displayLeftButtonPressed() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Left");
  display.println("Pressed!");
  display.display();
  delay(100);  // Debounce delay
}

// Two buttons, one to increase leg length and one to decrease
// Whenever there's an update to leg length, show the leg length screen, and show the increasing or decreasing leg length
// After 3 seconds of no input, switch back to efficiency display mode
// Or just replace the yellow optimal speed with the leg length value
void updateOptimal() {
  optimalSpeed = harmonic * sqrt(legLength * 9.81) / ((2 * 3.1415926535)*3.6);
  optimalPace = speedToPace(optimalSpeed);
}

float calculateEfficiency(float currentPace, float optimalPace) {
    float efficiency;
    // Calculate efficiency as the percentage difference from the optimal speed
    efficiency = (1 - (fabs(optimalPace - currentPace) / optimalPace)) * 100.0;
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
  float currentPace = speedToPace(gps.f_speed_kmph());
  char buffer[32];

  // Calculate efficiency
  float efficiency = calculateEfficiency(currentPace, optimalPace); 
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

