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
int leg_length;
int mass;
float d_speed = 1.0;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial1.println("$PMTK251,9600*17");  // Set baud rate to 9600
  delay(1000);
  Serial1.println("$PMTK220,200*2C");  // Set update rate to 200ms
  delay(1000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  delay(1000);
}

//Need to set up FSM between user input mode and speed checking/displaying, currently just printing speed/efficiency
void loop() {
  while (Serial1.available()) {
    char c = Serial1.read();
    if (gps.encode(c)) {
      displaySpeed();
    }
  }

}

// A function that will be used to update user parameters, will change  the ideal speed
// Need to read up on a good model to use to calculate ideal speed (resonant frequencies)
// void updateParameters(){
  
// }


//Measure and display speed
void displaySpeed() {
  float speedMps = gps.f_speed_kmph() / 3.6;
  char buffer[32]; // Buffer storing the messages to print to the screen
  
  // Calculate efficiency
  float efficiency = (1 - fabs(d_speed - speedMps) / d_speed) * 100.0;

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  sprintf(buffer, "%.2f m/s", d_speed);
  display.println(buffer);

  sprintf(buffer, "%.2f m/s", speedMps);
  display.println(buffer);

  display.setTextSize(2.5);
  sprintf(buffer, "%.2f%%", efficiency);
  display.println(buffer);

  // Extra message if there aren't enough satellites for proper speed data
  if (speedMps < 0) {
    display.setTextSize(1.5);
    display.println("Need...");
    display.println("Satellite...");
    display.println("m(_ _)m");
  }

  display.display();
}