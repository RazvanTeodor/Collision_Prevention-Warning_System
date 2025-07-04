// ===== Fișier: comunicare.cpp =====
#include "comunicare.h"
#include "config.h"
#include <SoftwareSerial.h>
#include <math.h>

static SoftwareSerial bt(PIN_BT_RX, PIN_BT_TX);

void initComunicare() {
  bt.begin(9600);
}

void trimiteDate(float yaw, float dist, SystemState senderState) {
  bt.print(yaw, 1);
  bt.print(',');
  bt.print(dist, 1);
  bt.print(',');
  bt.println((int)senderState); 
}

bool primesteDate(float &yawRemote, float &distRemote, SystemState &receivedState) {
  if (!bt.available()) return false;


  String s = bt.readStringUntil('\n');
  s.trim();

  int firstComma = s.indexOf(',');
  int secondComma = s.indexOf(',', firstComma + 1); 


 if (firstComma == -1 || secondComma == -1 ||
      firstComma == 0 || secondComma == s.length() - 1 ||
      firstComma == secondComma) { 
    return false;
  }

  yawRemote = s.substring(0, firstComma).toFloat();
  distRemote = s.substring(firstComma + 1, secondComma).toFloat();
  receivedState = (SystemState)s.substring(secondComma + 1).toInt(); 

   
  if (isnan(yawRemote) || isnan(distRemote) || isinf(yawRemote) || isinf(distRemote)) {
      return false;
  }
  

  return true;
}