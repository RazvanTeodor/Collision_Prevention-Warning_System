// ===== Fișier: motoare.cpp =====
#include "motoare.h"
#include "config.h"
#include <Arduino.h>

// Variabila globala pentru viteza motorului, definita aici si declarata extern in config.h
int g_current_motor_speed = 0;

void initMotoare() {
  pinMode(PIN_MOTOR_PWM, OUTPUT);
  pinMode(PIN_MOTOR_DIR_A, OUTPUT);

  noTone(PIN_BUZZER);

  setMotorSpeed(VITEZA_STOP);
   setBuzzerState(STATE_RUNNING_NORMAL);
}

void setMotorSpeed(int speed) {
  if (speed < 0) speed = 0;
  if (speed > 255) speed = 255;

  g_current_motor_speed = speed;

  if (speed == VITEZA_STOP) {
    digitalWrite(PIN_MOTOR_DIR_A, LOW);
   
    analogWrite(PIN_MOTOR_PWM, 0);
  } else {
    digitalWrite(PIN_MOTOR_DIR_A, HIGH);
   
    analogWrite(PIN_MOTOR_PWM, speed);
  }
}

// Această funcție activează buzzerul continuu doar pentru pericol
void setBuzzerState(SystemState alertType) { 
  
  const int DANGER_FREQUENCY = 3500; 
  // Verifică dacă starea curentă este una de pericol local sau remote
  if (alertType == STATE_LOCAL_ALERT_DANGER || alertType == STATE_REMOTE_ALERT_DANGER) {
    
    tone(PIN_BUZZER, DANGER_FREQUENCY);
  } else {
 
    noTone(PIN_BUZZER);
  }
}
int getCurrentSpeed() {
  return g_current_motor_speed;
}