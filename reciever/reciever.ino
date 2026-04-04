#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// --- NRF24L01 Configuration ---
RF24 radio(7, 8); // CE pin to D7, CSN pin to D8
const byte address[6] = "00001"; // MUST MATCH TRANSMITTER EXACTLY

// --- L298N Motor Driver Pins ---
#define ENA 5  // Left Motor Speed (PWM pin)
#define IN1 2  // Left Motor Forward
#define IN2 3  // Left Motor Backward
#define IN3 4  // Right Motor Forward
#define IN4 9  // Right Motor Backward
#define ENB 6  // Right Motor Speed (PWM pin)

// --- Data Structure for Transmission ---
struct Data_Package {
  char command[10]; // "FORWARD", "BACKWARD", "LEFT", "RIGHT", "STOP"
  int speed;        // 0 to 100% 
};
Data_Package data; 

// --- Failsafe Timer ---
unsigned long lastReceiveTime = 0; 
const unsigned long timeout = 500; // Stop car if no signal for 500 milliseconds

void setup() {
  Serial.begin(9600);
  
  // Set motor pins as outputs
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Initialize motors to STOP
  stopCar();

  Serial.println("Starting UNO Receiver & Motor Driver...");

  // Initialize the radio
  if (!radio.begin()) {
    Serial.println("FAIL: NRF24L01 not connected to UNO!");
    while (1); 
  }

  radio.openReadingPipe(0, address); 
  radio.setPALevel(RF24_PA_MIN); // Change to RF24_PA_MAX when taking the car outside
  radio.startListening(); 
  
  Serial.println("UNO Receiver Ready! Listening for data...");
}

void loop() {
  // Check if there is data arriving through the air
  if (radio.available()) {
    radio.read(&data, sizeof(Data_Package));
    lastReceiveTime = millis(); // Reset the failsafe timer
    
    // Map the 0-100% speed to the 0-255 PWM range the motor driver needs
    int mappedSpeed = map(data.speed, 0, 100, 0, 255);

    // Drive the car based on the command
    if (strcmp(data.command, "FORWARD") == 0) {
      driveForward(mappedSpeed);
    } 
    else if (strcmp(data.command, "BACKWARD") == 0) {
      driveBackward(mappedSpeed);
    } 
    else if (strcmp(data.command, "LEFT") == 0) {
      driveLeft(mappedSpeed);
    } 
    else if (strcmp(data.command, "RIGHT") == 0) {
      driveRight(mappedSpeed);
    } 
    else {
      stopCar();
    }

    // Optional: Print status for debugging
    Serial.print("Cmd: ");
    Serial.print(data.command);
    Serial.print(" | Speed: ");
    Serial.println(mappedSpeed);
  }

  // --- Safety Failsafe ---
  // If we haven't received a command in the last 500ms, stop the motors
  if (millis() - lastReceiveTime > timeout) {
    stopCar();
  }
}

// ==========================================
//          MOTOR CONTROL FUNCTIONS
// ==========================================

void driveForward(int speedPWM) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, speedPWM);
  analogWrite(ENB, speedPWM);
}

void driveBackward(int speedPWM) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, speedPWM);
  analogWrite(ENB, speedPWM);
}

void driveLeft(int speedPWM) {
  // To turn left, left motors go backward, right motors go forward (tank turn)
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, speedPWM);
  analogWrite(ENB, speedPWM);
}

void driveRight(int speedPWM) {
  // To turn right, left motors go forward, right motors go backward (tank turn)
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, speedPWM);
  analogWrite(ENB, speedPWM);
}

void stopCar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}