🛑 Collision Prevention Warning System (V2V Arduino Project)
This project is an embedded system designed to prevent vehicle collisions using Vehicle-to-Vehicle (V2V) communication and real-time alerts. Developed as a bachelor’s thesis at UNSTPB – ETTI, it detects obstacles in front of a vehicle and automatically transmits alerts to following vehicles via Bluetooth Low Energy (BLE), with no need for external infrastructure.

🔧 Technologies Used
Arduino UNO R3 – control unit for each vehicle

Ultrasonic Sensor HC-SR04 – obstacle distance measurement

Bluetooth Module HM-10 – BLE communication between vehicles

Gyroscope MPU6050 – orientation detection and alert filtering

TFT Display ST7789V – visual alert display

Passive Buzzer – audio alert for critical danger zone

C++ & Arduino IDE – logic implementation and programming

Proteus 8 Professional – early-stage simulation

🧠 System Logic
Continuous measurement of distance to obstacles;

Relative speed calculation to assess risk levels;

Alert generation based on danger zone (Safe / Warning / Critical);

BLE message transmission, including distance and orientation data;

Smart alert filtering using yaw and local distance.

⚠️ Risk Zones
Zone 1 – Safe: no alert;

Zone 2 – Warning: visual display + BLE alert sent;

Zone 3 – Critical: display + buzzer + BLE alert sent.

📚 Tested Scenarios
V1 in front, V2 behind – functional alert chain
V1 detects obstacle and sends alert to V2. Both vehicles respond properly (display and slowdown/stop).

V2 in front, V1 behind – role reversal
Any vehicle can become the sender if it detects a threat. System supports dynamic role switching.

Vehicles on opposite directions
Alert is not implemented by the vehicle on a different heading – yaw filtering prevents false alerts.

One vehicle stopped, one moving
Only the moving vehicle evaluates and possibly sends an alert. Stationary vehicles ignore incoming alerts.

Both vehicles moving slowly
V1 enters the warning zone while V2 follows closely.
However, relative speed is below threshold (e.g., traffic jam, parking).
No alert is triggered. The system stays inactive in low-speed scenarios to avoid alert spam.
