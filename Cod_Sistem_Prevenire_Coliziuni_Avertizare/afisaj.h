#ifndef AFISAJ_H
#define AFISAJ_H

#include "config.h"

void initAfisaj();

void afisareStareCurenta(float yawLocal, bool hasRemote, float distLocal, float distRemote, SystemState state, RelativePosition relPos, float currentRelativeSpeedLocal = 0.0f);

void afisareValoriPermanente(float yawLocal, bool hasRemote, float distLocal, float distRemote, RelativePosition relPos, float currentRelativeSpeedLocal);

#endif // AFISAJ_H