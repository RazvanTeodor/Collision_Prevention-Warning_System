// ===== Fișier: motoare.h =====
#ifndef MOTOARE_H
#define MOTOARE_H

#include "config.h"

void initMotoare();
void setMotorSpeed(int speed);
void setBuzzerState(SystemState alertType);
int getCurrentSpeed(); // Functie pentru a obtine viteza curenta a motoarelor

#endif // MOTOARE_H