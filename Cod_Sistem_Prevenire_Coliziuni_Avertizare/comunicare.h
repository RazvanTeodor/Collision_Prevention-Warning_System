// ===== Fișier: comunicare.h =====
#ifndef COMUNICARE_H
#define COMUNICARE_H

#include "config.h"

void initComunicare();
void trimiteDate(float yaw, float dist,  SystemState senderState);
bool primesteDate(float &yawRemote, float &distRemote, SystemState &receivedState);

#endif // COMUNICARE_H