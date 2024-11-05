# In-progress
🚗 Collision Prevention Warning System

Description:
This project implements a vehicle collision prevention system that detects obstacles in real-time and automatically initiates warnings for both the detecting vehicle (V1) and a nearby vehicle (V2) using Bluetooth technology.

✨ Key Features
📏 Distance Detection and Classification
An ultrasonic sensor continuously measures the distance to obstacles, categorizing them into three zones:

🟢 Safe Zone
🟡 Warning Zone
🔴 Danger Zone
⚠️ Warning Signals

In the Warning Zone: V1 displays a message on the screen to alert the driver.
In the Danger Zone: The system activates both a visual alert and a buzzer to signal high risk.

📲 Alert Transmission to Nearby Vehicle
If there is a high risk of collision, V1 sends alerts via Bluetooth to V2, providing it with similar visual and audible warnings.

🛠️ Technologies Used
Hardware: Arduino Uno, Bluetooth modules, ultrasonic sensors, buzzer, displays.
Software: Developed in C/C++ on the Arduino IDE.
