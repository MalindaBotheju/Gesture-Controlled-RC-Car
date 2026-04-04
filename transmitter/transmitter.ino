#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "I2Cdev.h"
#include "MPU6050.h"

// --- OLED Display Configuration ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- NRF24L01 Configuration ---
RF24 radio(7, 8); // CE pin to D7, CSN pin to D8
const byte address[6] = "00001"; // Must match receiver

// --- MPU6050 Configuration ---
MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;

// --- Data Structure for Transmission ---
// The receiver MUST use this exact same structure
struct Data_Package {
  char command[10]; // "FORWARD", "BACKWARD", "LEFT", "RIGHT", "STOP"
  int speed;        // 0 to 100% mapped from tilt angle
};
Data_Package data;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // 1. Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("OLED init failed"));
    for(;;); 
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println("System Booting...");
  display.display();

  // 2. Initialize MPU6050
  mpu.initialize();
  if (!mpu.testConnection()) {
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("MPU6050 FAILED!");
    display.display();
    while(1);
  }

  // --- YOUR EXACT CALIBRATION OFFSETS ---
  mpu.setXAccelOffset(-518);
  mpu.setYAccelOffset(-4040);
  mpu.setZAccelOffset(1328);
  mpu.setXGyroOffset(76);
  mpu.setYGyroOffset(-14);
  mpu.setZGyroOffset(40);

  // 3. Initialize NRF24L01
  if (!radio.begin()) {
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("NRF24L01 FAILED!");
    display.display();
    while(1);
  }
  
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN); // Use MIN for testing, MAX for real-world
  radio.stopListening(); // Set as Transmitter

  display.clearDisplay();
  display.setCursor(0, 20);
  display.setTextSize(2);
  display.println("READY!");
  display.display();
  delay(1000);
}

void loop() {
  // Read raw acceleration data (we ignore gyro here because we only need tilt)
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Map the raw accelerometer values (-16384 to 16384 is roughly +/- 1g)
  // We map it to -100 to 100 for easier math
  int xTilt = map(ax, -16384, 16384, -100, 100); 
  int yTilt = map(ay, -16384, 16384, -100, 100);

  // Deadzone so the car doesn't jitter when your hand naturally shakes
  int deadzone = 25; 

  // Reset speed to 0 by default
  data.speed = 0;

  // Determine Direction and Speed based on tilt
  // Note: Depending on how the MPU is mounted on your glove, 
  // you might need to swap xTilt and yTilt, or swap FORWARD/BACKWARD.
  
  if (yTilt > deadzone) {
    strcpy(data.command, "FORWARD");
    data.speed = map(yTilt, deadzone, 100, 0, 100); // Calculate throttle
  } 
  else if (yTilt < -deadzone) {
    strcpy(data.command, "BACKWARD");
    data.speed = map(yTilt, -deadzone, -100, 0, 100); 
  } 
  else if (xTilt > deadzone) {
    strcpy(data.command, "RIGHT");
    data.speed = map(xTilt, deadzone, 100, 0, 100); 
  } 
  else if (xTilt < -deadzone) {
    strcpy(data.command, "LEFT");
    data.speed = map(xTilt, -deadzone, -100, 0, 100); 
  } 
  else {
    strcpy(data.command, "STOP");
    data.speed = 0;
  }

  // Constrain speed just in case the math overshoots (safety first!)
  data.speed = constrain(data.speed, 0, 100);

  // Send the data package over the radio
  bool success = radio.write(&data, sizeof(Data_Package));

  // --- Update the OLED Screen ---
  display.clearDisplay();
  
  // 1. Show Direction (Command)
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(data.command);

  // 2. Show Speed (Magnitude)
  display.setTextSize(1);
  display.setCursor(0, 25);
  display.print("Car Speed: ");
  display.print(data.speed);
  display.println("%");

  // 3. Show "Velocity" (Direction + Speed combined)
  display.setCursor(0, 35);
  display.print("Velocity: ");
  if (strcmp(data.command, "STOP") == 0) {
    display.print("0");
  } else {
    display.print(data.speed);
    display.print("% ");
    // Print just the first letter of direction for space (F, B, L, R)
    display.print(data.command[0]); 
  }

  // 4. Show Transmission Status
  display.setCursor(0, 55);
  if (success) {
    display.print("Signal: TX OK");
  } else {
    display.print("Signal: TX FAIL");
  }

  display.display();
  
  // Wait a short moment before reading again
  delay(50); 
}