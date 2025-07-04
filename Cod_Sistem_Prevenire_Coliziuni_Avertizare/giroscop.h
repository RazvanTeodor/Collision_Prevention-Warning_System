// ===== Fișier: giroscop.h =====
#ifndef GIROSCOP_H
#define GIROSCOP_H

#include "config.h" // Aici va fi inclus si YAW_RATE_THRESHOLD prin #define

void initGiroscop();
float calcYaw(float dt);

#endif // GIROSCOP_H