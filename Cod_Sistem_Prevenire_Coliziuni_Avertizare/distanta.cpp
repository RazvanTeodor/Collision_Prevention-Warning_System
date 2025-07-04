// ===== Fișier: distanta.cpp =====
#include "distanta.h"
#include "config.h"
#include <Arduino.h>

static float lastValidDist = 0.0f; // Variabila statica pentru ultima distanta valida

void initDistanta() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  lastValidDist = 100.0f; // Inițializăm cu o valoare sigură, presupunem că nu e obstacol aproape
}

float masurareDistanta() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  long duration = pulseIn(PIN_ECHO, HIGH, 30000); // Timeout la 30ms (max 5m)
  float dist = duration * 0.01715;  // cm

  // Filtrare mai robusta
  if (dist > 0 && dist <= 400) { // Consideram valori valide intre 0 si 400 cm (4m)
    lastValidDist = dist;
    return dist;
  } else {
    // Daca citirea este 0 sau peste 400, returnam ultima valoare valida cunoscuta.
    // Aceasta previne trecerile false in "siguranta" din cauza citirilor eronate.
    return lastValidDist;
  }
}