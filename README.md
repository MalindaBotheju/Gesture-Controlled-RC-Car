# 🏎️ Gesture-Controlled RC Car with MPU6050 & NRF24L01

A fully functional, gesture-controlled robotic car built with Arduino. This project uses a hand-mounted MPU6050 accelerometer to detect tilt, transmits the data wirelessly via NRF24L01 radio modules, and drives a 4-wheel robot car with proportional speed control. 

An onboard OLED screen on the transmitter displays real-time telemetry, including direction, throttle percentage, and transmission status.

## ✨ Features
* **Intuitive Gesture Control:** Tilt your hand forward, backward, left, or right to drive the car.
* **Proportional Throttle:** The steeper you tilt your hand, the faster the car goes (0-100% speed mapping).
* **Live Telemetry Dashboard:** An OLED screen on the remote shows current direction, speed percentage, and radio connection status.
* **Wireless Failsafe System:** If the car loses connection with the remote for more than 500ms, it automatically stops all motors to prevent runaways.
* **Tank Steering:** For tight turns, the left and right wheels spin in opposite directions to rotate the car in place.

---

## 🛠️ Hardware Requirements

### Transmitter (The Glove/Remote)
* 1x Arduino Nano
* 1x NRF24L01 Wireless Transceiver Module
* 1x NRF24L01 Voltage Adapter Base (Highly recommended to provide stable 3.3V)
* 1x MPU6050 Accelerometer/Gyroscope Sensor
* 1x 0.96" OLED Display (I2C / SSD1306)
* Breadboard & Jumper Wires
* Portable Power Source (e.g., 9V battery or power bank)

### Receiver (The Car)
* 1x Arduino Uno
* 1x NRF24L01 Wireless Transceiver Module
* 1x L298N Motor Driver
* 4x DC Gear Motors with Wheels
* 1x 11.1V Power Source (e.g., 3x 3.7V 18650 batteries in series) - *Must be high-current!*
* Robot Chassis

---

## 🔌 Wiring Schematics

### Transmitter (Arduino Nano)
| Component | Pin | Arduino Nano Pin | Notes |
| :--- | :--- | :--- | :--- |
| **OLED & MPU6050** | VCC | 5V | |
| | GND | GND | |
| | SDA | A4 | I2C Data |
| | SCL | A5 | I2C Clock |
| **NRF24L01** | VCC | 3.3V (or 5V if using Adapter) | *Do not power directly from Nano 3.3V without a capacitor!* |
| | GND | GND | |
| | CE | D7 | |
| | CSN | D8 | |
| | MOSI | D11 | |
| | MISO | D12 | |
| | SCK | D13 | |

### Receiver (Arduino Uno)
| Component | Pin | Arduino Uno Pin | Notes |
| :--- | :--- | :--- | :--- |
| **NRF24L01**| VCC | 3.3V | The Uno has a strong enough 3.3V regulator for the radio. |
| | GND | GND | |
| | CE | D7 | |
| | CSN | D8 | |
| | MOSI | D11 | |
| | MISO | D12 | |
| | SCK | D13 | |
| **L298N** | 12V | Battery Positive (+) | Recommend 11.1V (3x 3.7V batteries in series). |
| | GND | Battery (-) & Uno GND | **CRITICAL: Must share ground with Arduino!** |
| | 5V Output| Uno 5V Pin | Powers the Arduino. *Do not plug in USB while battery is ON.* |
| | ENA | D5 | Remove black jumper cap. |
| | IN1 | D2 | Left Forward |
| | IN2 | D3 | Left Backward |
| | IN3 | D4 | Right Forward |
| | IN4 | D9 | Right Backward |
| | ENB | D6 | Remove black jumper cap. |

---

## 💻 Software Setup

### Required Libraries
You will need to install the following libraries via the Arduino Library Manager (`Sketch` -> `Include Library` -> `Manage Libraries`):
* `RF24` by TMRh20
* `MPU6050` by Electronic Cats (or standard I2Cdev implementation)
* `Adafruit GFX Library` by Adafruit
* `Adafruit SSD1306` by Adafruit

### Installation
1.  Clone this repository to your local machine.
2.  Open the `Transmitter` folder and upload the code to your Arduino Nano.
3.  Open the `Receiver` folder and upload the code to your Arduino Uno.
4.  *Note: Make sure your battery pack is unplugged from the L298N before connecting your Uno to the computer via USB to upload code!*

---

## ⚠️ Common Troubleshooting (Lessons Learned)

* **NRF24L01 FAILS to Initialize on Nano:** The Arduino Nano's onboard 3.3V pin is notoriously weak. If your radio fails to start, use a dedicated NRF24L01 Voltage Adapter board connected to the Nano's 5V pin, or solder a 10µF - 100µF capacitor across the 3.3V and GND pins.
* **Motors Whine but Won't Spin:** DC motors require high current. Standard 9V "smoke detector" batteries will not work. Use three 3.7V batteries in series (11.1V total) to power the L298N. Also, ensure you tilt the transmitter steeply enough to push the PWM speed above 30%.
* **Car Drives Forward, but Spins in a Circle in Reverse:** Check your jumper wires between the Arduino and the L298N (IN1, IN2, IN3, IN4). If one side fails to reverse, the wire responsible for that direction (e.g., IN2 or IN4) is likely broken internally or loose. Replace the jumper wire.
* **Random Twitches / Fails to Move:** Ensure there is a wire connecting the GND terminal of the L298N directly to a GND pin on the Arduino Uno. The PWM signals cannot complete their circuit without a shared ground.