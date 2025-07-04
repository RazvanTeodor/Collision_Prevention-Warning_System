#include "afisaj.h"
#include "config.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

static Adafruit_ST7789 tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);

// Declarare globala pentru a stoca ultima viteza afisata (pentru a evita rescrieri inutile)
static float lastDisplayedRelativeSpeedLocal = -1.0f;

void initAfisaj() {
  tft.init(240, 320);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
}


void afisareStareCurenta(float yawLocal, bool hasRemote, float distLocal, float distRemote, SystemState state, RelativePosition relPos, float currentRelativeSpeedLocal) {
  uint16_t bgColor;
  uint16_t textColor;
  String mainMessage1 = "";
  String mainMessage2 = "";
  String infoMessage = "";


  // Setează culorile de fundal și text și mesajele principale
  switch (state) {
    case STATE_WAITING_STATIONARE:
      bgColor = ST77XX_BLUE; textColor = ST77XX_WHITE;
      mainMessage1 = "STATIONEZ";
      break;
    case STATE_LOCAL_ALERT_DANGER:
      bgColor = ST77XX_RED; textColor = ST77XX_WHITE;
      mainMessage1 = "PERICOL!";
      infoMessage = "Local: " + String(distLocal, 1) + " cm";
      break;
    case STATE_LOCAL_ALERT_CAUTION:
      bgColor = ST77XX_ORANGE; textColor = ST77XX_BLACK;
      mainMessage1 = "ATENTIE!";
      infoMessage = "Local: " + String(distLocal, 1) + " cm";
      break;
    case STATE_REMOTE_ALERT_DANGER:
      bgColor = ST77XX_MAGENTA; textColor = ST77XX_WHITE;
      mainMessage1 = "PRIMIT";
      mainMessage2 = "PERICOL!";
      infoMessage = "Partener: " + String(distRemote, 1) + " cm";
      break;
    case STATE_REMOTE_ALERT_CAUTION:
      bgColor = ST77XX_MAGENTA; textColor = ST77XX_WHITE;
      mainMessage1 = "PRIMIT";
      mainMessage2 = "ATENTIE!";
      infoMessage = "Partener: " + String(distRemote, 1) + " cm";
      break;
    
    case STATE_DIFFERENT_DIRECTIONS: // Este o stare validă, dar pe fundal verde
      bgColor = ST77XX_GREEN; textColor = ST77XX_BLACK;
      mainMessage1 = "DIRECTII";
      mainMessage2 = "DIFERITE";
      infoMessage = "Local OK";
      break;
    case STATE_RUNNING_NORMAL: // Starea de baza "Siguranta"
      bgColor = ST77XX_GREEN; textColor = ST77XX_BLACK;
      mainMessage1 = "SIGURANTA";
      infoMessage = "Local: " + String(distLocal, 1) + " cm";
      break;
    default: // Fallback
      bgColor = ST77XX_BLACK; textColor = ST77XX_WHITE;
      mainMessage1 = "EROARE STARE";
      break;
  }

  tft.fillScreen(bgColor);
  tft.setTextColor(textColor);

  // === Blocul de afisaj permanent (sus) ===
  tft.setTextSize(2);
  
  // 1. Yaw
  tft.setCursor(5, 5);
  tft.print("Yaw: ");
  tft.print(yawLocal, 1);
  tft.println(" deg");

  // 2. Viteza, sub Yaw
  tft.setCursor(5, 30); 
  tft.print("Viteza: ");
  tft.print(abs(currentRelativeSpeedLocal), 2); // Afisam valoarea absoluta
  tft.println(" m/s");

  // 3. PARTENER, sub Viteza
  tft.setCursor(5, 55); 
  tft.print("PARTENER: ");
  if (hasRemote) {
    switch (relPos) {
      case IN_FRONT_PARTNER:    tft.println("IN FATA"); break;
      case BEHIND_PARTNER:      tft.println("IN SPATE"); break;
      case DIFFERENT_DIRECTIONS: tft.println("DIFERITE"); break;
      default:                  tft.println("NECUNOSCUT"); break;
    }
  } else {
      tft.println("NECUNOSCUT");
  }
  // Linia de separare
  tft.drawLine(0, 80, tft.width(), 80, ST77XX_WHITE); 

  // === Afisaj principal (centru) 
  tft.setTextSize(3);
  int16_t x1, y1;
  uint16_t w1, h1;
  
  // Mesajul principal de stare
  tft.getTextBounds(mainMessage1.c_str(), 0, 0, &x1, &y1, &w1, &h1);
  tft.setCursor(tft.width() / 2 - w1 / 2, tft.height() / 2 - 20); 
  tft.print(mainMessage1);
  
  if (mainMessage2 != "") {
    tft.getTextBounds(mainMessage2.c_str(), 0, 0, &x1, &y1, &w1, &h1);
    tft.setCursor(tft.width() / 2 - w1 / 2, tft.height() / 2 + 10); // Sub primul mesaj principal
    tft.print(mainMessage2);
  }
  
  tft.setTextSize(2);
 
  if (infoMessage != "") {
    tft.getTextBounds(infoMessage.c_str(), 0, 0, &x1, &y1, &w1, &h1);
    tft.setCursor(tft.width() / 2 - w1 / 2, tft.height() / 2 + 60);
    tft.print(infoMessage);
  }

  // Zona de jos pentru mesaje 
  tft.fillRect(0, tft.height() - 20, tft.width(), 20, bgColor);
}


// Functia afisareValoriPermanente
void afisareValoriPermanente(float yawLocal, bool hasRemote, float distLocal, float distRemote, RelativePosition relPos, float currentRelativeSpeedLocal) {
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(2);

  int16_t x1, y1;
  uint16_t w1, h1;

  // 1. Afisare Yaw
  String yawString = "Yaw: " + String(yawLocal, 1) + " deg";
  tft.getTextBounds(yawString.c_str(), 0, 0, &x1, &y1, &w1, &h1);
  tft.fillRect(5, 5, w1 + 10, h1, ST77XX_BLACK);
  tft.setCursor(5, 5);
  tft.print(yawString);

  // 2. Afisarea vitezei sub Yaw
  String speedString = "Viteza: " + String(abs(currentRelativeSpeedLocal), 2) + " m/s";
  tft.getTextBounds(speedString.c_str(), 0, 0, &x1, &y1, &w1, &h1);
  tft.fillRect(5, 30, w1 + 10, h1, ST77XX_BLACK); 
  tft.setCursor(5, 30); 
  tft.print(speedString);

  // 3. Afisare PARTENER
  String partnerString;
  if (hasRemote) {
    switch (relPos) {
      case IN_FRONT_PARTNER:    partnerString = "PARTENER: IN FATA"; break;
      case BEHIND_PARTNER:      partnerString = "PARTENER: IN SPATE"; break;
      case DIFFERENT_DIRECTIONS: partnerString = "PARTENER: DIFERITE"; break;
      default:                  partnerString = "PARTENER: NECUNOSCUT"; break;
    }
  } else {
      partnerString = "PARTENER: NECUNOSCUT";
  }
  tft.getTextBounds(partnerString.c_str(), 0, 0, &x1, &y1, &w1, &h1);
  tft.fillRect(5, 55, w1 + 10, h1, ST77XX_BLACK); 
  tft.setCursor(5, 55); 
  tft.print(partnerString);
  
  uint16_t currentMainBgColor;
  uint16_t currentMainTextColor;

  switch (g_current_system_state) {
    case STATE_LOCAL_ALERT_DANGER: currentMainBgColor = ST77XX_RED; currentMainTextColor = ST77XX_WHITE; break;
    case STATE_LOCAL_ALERT_CAUTION: currentMainBgColor = ST77XX_ORANGE; currentMainTextColor = ST77XX_BLACK; break;
    case STATE_REMOTE_ALERT_DANGER:
    case STATE_REMOTE_ALERT_CAUTION: currentMainBgColor = ST77XX_MAGENTA; currentMainTextColor = ST77XX_WHITE; break;
    case STATE_RUNNING_NORMAL:
    
    case STATE_DIFFERENT_DIRECTIONS:
      currentMainBgColor = ST77XX_GREEN; currentMainTextColor = ST77XX_BLACK; break;
    case STATE_WAITING_STATIONARE: return; 
    default: currentMainBgColor = ST77XX_BLACK; currentMainTextColor = ST77XX_WHITE; break;
  }

  tft.setTextColor(currentMainTextColor, currentMainBgColor);
  tft.setTextSize(2);


  String textToDisplay = "";
  if (g_current_system_state == STATE_RUNNING_NORMAL  || g_current_system_state == STATE_DIFFERENT_DIRECTIONS || g_current_system_state == STATE_LOCAL_ALERT_CAUTION || g_current_system_state == STATE_LOCAL_ALERT_DANGER) {
    textToDisplay = "Local: " + String(distLocal, 1) + " cm";
    tft.getTextBounds(textToDisplay.c_str(), 0, 0, &x1, &y1, &w1, &h1);
    tft.fillRect(tft.width() / 2 - w1 / 2, tft.height() / 2 + 40, w1, h1, currentMainBgColor); 
    tft.setCursor(tft.width() / 2 - w1 / 2, tft.height() / 2 + 40); 
    tft.print(textToDisplay);
  } else if (g_current_system_state == STATE_REMOTE_ALERT_DANGER || g_current_system_state == STATE_REMOTE_ALERT_CAUTION) {
    textToDisplay = "Partener: " + String(distRemote, 1) + " cm";
    tft.getTextBounds(textToDisplay.c_str(), 0, 0, &x1, &y1, &w1, &h1);
    tft.fillRect(tft.width() / 2 - w1 / 2, tft.height() / 2 + 70, w1, h1, currentMainBgColor); 
    tft.setCursor(tft.width() / 2 - w1 / 2, tft.height() / 2 + 70); 
    tft.print(textToDisplay);
  }

  // Curatam zona de jos
  tft.fillRect(0, tft.height() - 20, tft.width(), 20, currentMainBgColor);
}