// ===== Fișier: giroscop.cpp =====
#include "giroscop.h"
#include "config.h" 
#include <Wire.h>
#include <MPU6050.h>

static MPU6050 mpu;
static float yawLocal    = 0;
static float offsetGz    = 0;
static float prevRateZ   = 0;

void initGiroscop() {
  Wire.begin();
  mpu.initialize();
  while (!mpu.testConnection()) { delay(100); }
  
  long sum = 0;
  int16_t gx, gy, gz;
  for (int i = 0; i < GYRO_CALIB_SAMPLES; i++) {
    mpu.getRotation(&gx, &gy, &gz);
    sum += gz;
    delay(10);
  }
  offsetGz = sum / (float)GYRO_CALIB_SAMPLES;
}

float calcYaw(float dt) {
  int16_t gx, gy, gz;
  mpu.getRotation(&gx, &gy, &gz);
  float rawRateZ = (gz - offsetGz) / GYRO_SENSITIVITY;
  float rateZ = SMOOTHING_ALPHA * prevRateZ + (1 - SMOOTHING_ALPHA) * rawRateZ;
  prevRateZ = rateZ;
  
  // Aplica logica de "Dead Zone" pentru Yaw
  if (abs(rateZ) > YAW_RATE_THRESHOLD) { // YAW_RATE_THRESHOLD este acum un #define vizibil
    yawLocal += rateZ * (dt / 1000.0f);
  }
  // else: yawLocal ramane la fel, ignora zgomotul sub prag

  if (yawLocal < 0) yawLocal += 360;
  if (yawLocal >= 360) yawLocal -= 360;
  return yawLocal;
}