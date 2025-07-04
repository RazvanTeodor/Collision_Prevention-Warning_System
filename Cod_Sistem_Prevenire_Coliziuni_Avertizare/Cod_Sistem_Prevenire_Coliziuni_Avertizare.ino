// ===== Fisier: ProiectColiziuni.ino =====
#include <Arduino.h>
#include "config.h"
#include "distanta.h"
#include "giroscop.h"
#include "comunicare.h"
#include "afisaj.h"
#include "motoare.h"


SystemState g_current_system_state = STATE_WAITING_STATIONARE;

// ************ DECLARAREA VARIABILELOR GLOBALE ************
float lastYawLocal = 0;
float lastDistLocal = 0;
float lastDistRemote = 0;
bool hasRemoteData = false;
unsigned long startTime = 0;

SystemState lastDisplayedState = (SystemState)-1;
float lastDisplayedYaw = -1.0;
float lastDisplayedDistLocal = -1.0;
float lastDisplayedDistRemote = -1.0;
RelativePosition lastDisplayedRelPos = UNKNOWN;
unsigned long lastDisplayUpdateMillis = 0;
float lastDisplayedRelativeSpeedLocal = -1.0f; // pentru afisajul vitezei

unsigned long lastRemoteAlertActiveTime = 0; 
SystemState lastRemoteAlertState = STATE_RUNNING_NORMAL;
// Variabila pentru a înregistra timpul ultimei recepții valide de date remote
unsigned long lastRemoteDataReceiveTime = 0;

// Variabile pentru calculul vitezei relative (declarațiile concrete aici)
float g_previous_dist_local = 0.0f;
unsigned long g_previous_dist_time = 0UL;
float g_smoothed_relative_speed_local = 0.0f; // Variabila pentru viteza filtrata

// Variabilă care reține dacă viteza a fost suficient de mare pentru a declanșa alerta.
// Nu comută imediat înapoi la FALSE pentru a evita oscilațiile — se aplică o marjă de siguranță (histerezis).
static bool g_speed_is_sufficient_for_alert = false;
// *******************************************************************


void setup() {
  Serial.begin(9600);
  initDistanta();
  initComunicare();
  initGiroscop();
  initAfisaj();
  initMotoare();

  startTime = millis(); // Înregistrează timpul de start
}

void loop() {
  unsigned long now = millis();
  static unsigned long lastLogicMillis = now;
  unsigned long dt = now - lastLogicMillis;

  // Rulăm logica principală la INTERVAL_MS
  if (dt < INTERVAL_MS) {
    return;
  }
  lastLogicMillis = now;

  // Declararea variabilelor la începutul funcției loop() pentru scope corect
  SystemState nextState = STATE_RUNNING_NORMAL; // Starea implicită este SIGURANȚĂ
  RelativePosition currentRelPos = UNKNOWN;


  // 1. Citiri Locale
  float yawLocal = calcYaw(dt);
  float distLocal = masurareDistanta(); 


  // Calculul vitezei relative locale brute (rata de schimbare a distantei in m/s)
  float rawRelativeSpeedLocal = 0.0f; // Viteza bruta, inainte de filtrare (pozitiv daca ne apropiam)

  // Asigurăm că avem date anterioare și un interval de timp suficient pentru un calcul valid
  if (g_previous_dist_time > 0 && (now - g_previous_dist_time) >= MIN_TIME_FOR_VELOCITY_CALC) {
    float distChange = g_previous_dist_local - distLocal; // Delta d (pozitiv daca distanta scade -> ne apropiam)
    unsigned long timeChange = now - g_previous_dist_time; // Delta t in ms

    // Convertim cm/ms in m/s
    rawRelativeSpeedLocal = (distChange / timeChange) * (1000.0f / 100.0f); // (cm/ms) * (1000ms/s) / (100cm/m) = m/s
    
    // Pentru logica alertelor, ne intereseaza doar apropierea. Daca ne indepartam, viteza e 0 din perspectiva alertei.
    if (rawRelativeSpeedLocal < 0) rawRelativeSpeedLocal = 0.0f;
  } else {
      rawRelativeSpeedLocal = 0.0f;
  }

  // Aplicam filtrare pe viteza relativa bruta
  g_smoothed_relative_speed_local = (SPEED_SMOOTHING_ALPHA * rawRelativeSpeedLocal) + 
                                    ((1.0f - SPEED_SMOOTHING_ALPHA) * g_smoothed_relative_speed_local);

  //Actualizeaza valorile anterioare pentru urmatorul calcul (după ce am folosit distLocal în calcul)
  g_previous_dist_local = distLocal;
  g_previous_dist_time = now;


  // Filtrare distanta locala (ignora valori aberante, desi functia masurareDistanta face deja asta)
  lastDistLocal = distLocal; 
  lastYawLocal = yawLocal;

  // 3. Primește date remote
  float yawRemote_temp, distRemote_temp;
  SystemState remoteState_temp;
  bool dataJustReceived = primesteDate(yawRemote_temp, distRemote_temp, remoteState_temp);

  if (dataJustReceived) {
    lastDistRemote = distRemote_temp;
    lastRemoteAlertState = remoteState_temp;
    lastRemoteDataReceiveTime = now;
    hasRemoteData = true;
  } else {
    if (now - lastRemoteDataReceiveTime > REMOTE_DATA_TIMEOUT_MS) {
      hasRemoteData = false;
      lastDistRemote = 9999.0f; // Valoare mare, pentru a nu declansa alerte de distanta remote expirata
      lastRemoteAlertState = STATE_RUNNING_NORMAL; // Starea partenerului este normala daca nu primim date
    }
  }

  // 4. Determină dacă vehiculul local este în mișcare (pentru relevanța alertelor remote)

  // Folosim abs() pentru a detecta miscare indiferent de directie (apropiere sau indepartare)
  bool localMoving = (abs(g_smoothed_relative_speed_local) * 100.0f > MIN_DIST_CHANGE_FOR_MOVEMENT); // Convertim m/s la cm/s pentru comparatie


  //5. LOGICA STĂRILOR ȘI PRIORITĂȚILOR (Determină nextState)

  // 5.1 Prioritate Maxima: Faza de Start-up
  if (now - startTime < WAIT_START_MS) {
    nextState = STATE_WAITING_STATIONARE;
  }
// 5.2 Prioritate Absolută: Alerte Locale (Histerezis: Intrare cu Viteza, Ieșire cu Distanța)
  else {
    // Condiții de distanță pure (nu includ viteza), pentru a fi folosite în histerezisul pe distanță
    bool distanceIsDanger = (lastDistLocal < PRAG_PERICOL_ENTRY_LOCAL);
    bool distanceIsCaution = (lastDistLocal >= PRAG_ATENTIE_ENTRY_MIN_LOCAL && lastDistLocal <= PRAG_ATENTIE_ENTRY_MAX_LOCAL);
    bool distanceIsSafe = (lastDistLocal > PRAG_ATENTIE_EXIT_MAX_LOCAL); // Noua conditie pentru "distanta sigura"

    // Logica pentru condiția de viteză suficientă pentru alertă
    // g_speed_is_sufficient_for_alert devine TRUE doar dacă viteza depășește pragul ENTRY
    // g_speed_is_sufficient_for_alert devine FALSE doar dacă viteza scade sub pragul EXIT
    if (g_speed_is_sufficient_for_alert == false) {
        if (g_smoothed_relative_speed_local >= MIN_APPROACH_SPEED_FOR_ALERT_ENTRY) {
            g_speed_is_sufficient_for_alert = true;
        }
    } else { // g_speed_is_sufficient_for_alert == true
        if (g_smoothed_relative_speed_local < MIN_APPROACH_SPEED_FOR_ALERT_EXIT) {
            g_speed_is_sufficient_for_alert = false;
        }
    }

    // Acum, luăm decizia pentru nextState bazându-ne pe starea curentă
    switch (g_current_system_state) {
        case STATE_LOCAL_ALERT_DANGER:
            // Odată în DANGER, ieși doar când distanța este suficient de mare (histerezis pe distanță)
            if (lastDistLocal >= PRAG_PERICOL_EXIT_LOCAL) {
                // Dacă am ieșit din zona de pericol, dar suntem încă în zona de atenție
                if (distanceIsCaution) {
                    nextState = STATE_LOCAL_ALERT_CAUTION;
                } else { // Am ieșit complet din ambele zone de alertă
                    nextState = STATE_RUNNING_NORMAL;
                }
            } else {
                nextState = STATE_LOCAL_ALERT_DANGER; // Rămâi în DANGER
            }
            break;

        case STATE_LOCAL_ALERT_CAUTION:
            // Dacă condițiile de DANGER sunt îndeplinite (distanță ȘI viteză)
            if (distanceIsDanger && g_speed_is_sufficient_for_alert) {
                nextState = STATE_LOCAL_ALERT_DANGER; //PERICOL
            }
            // Altfel, ieși din CAUTION doar când distanța este suficient de mare (histerezis pe distanță)
            else if (lastDistLocal > PRAG_ATENTIE_EXIT_MAX_LOCAL) {
                nextState = STATE_RUNNING_NORMAL;
            } else {
                nextState = STATE_LOCAL_ALERT_CAUTION; // Rămâi în CAUTION
            }
            break;

        case STATE_RUNNING_NORMAL: // Include toate celelalte stări non-locale-alert
        case STATE_WAITING_STATIONARE: // Nu ar trebui sa intre aici daca e inca wait_start_ms
        case STATE_REMOTE_ALERT_DANGER:
        case STATE_REMOTE_ALERT_CAUTION:
        case STATE_DIFFERENT_DIRECTIONS:
            // Intră în alertă DANGER dacă distanța e DANGER ȘI viteza e suficientă
            if (distanceIsDanger && g_speed_is_sufficient_for_alert) {
                nextState = STATE_LOCAL_ALERT_DANGER;
            }
            // Altfel, intră în alertă CAUTION dacă distanța e CAUTION ȘI viteza e suficientă
            else if (distanceIsCaution && g_speed_is_sufficient_for_alert) {
                nextState = STATE_LOCAL_ALERT_CAUTION;
            }
            // Altfel, păstrează nextState calculat anterior (care e STATE_RUNNING_NORMAL by default sau o alertă remote)
            break;
    }

    // Nota: Blocul 5.3 pentru alertele remote și prioritizarea lor
    
    // se aplică *după* ce nextState a fost determinat pentru alertele locale.
    // Acum, nextState va fi determinat inițial de alertele locale.
    // Dacă nextState este acum o alertă LOCALĂ, blocul 5.3 o va lăsa așa, conform priorității.
    // Dacă nextState nu este o alertă LOCALĂ (ex: NORMAL), atunci blocul 5.3 va evalua alertele remote.
    // Această prioritizare este implicită prin ordinea de execuție.
    
    // Asiguram ca STATE_WAITING_STATIONARE are prioritate maxima absoluta 
    if (now - startTime < WAIT_START_MS) {
        nextState = STATE_WAITING_STATIONARE;
    } else {
       
        if (nextState != STATE_LOCAL_ALERT_DANGER && nextState != STATE_LOCAL_ALERT_CAUTION) {
            if (hasRemoteData) { 

                float deltaYaw = yawLocal - yawRemote_temp;
                if (deltaYaw > 180) deltaYaw -= 360;
                if (deltaYaw < -180) deltaYaw += 360;
                float absYaw = abs(deltaYaw);

                if (absYaw <= PRAG_ACEEASI_DIRECTIE_STRICT || absYaw <= PRAG_ACEEASI_DIRECTIE_LARG) {
                  bool localInFata = (lastDistLocal < lastDistRemote);
                  currentRelPos = localInFata ? IN_FRONT_PARTNER : BEHIND_PARTNER;
                  
                  if (currentRelPos == BEHIND_PARTNER && localMoving) { 

                    if (lastRemoteAlertState == STATE_LOCAL_ALERT_DANGER) {
                        nextState = STATE_REMOTE_ALERT_DANGER;
                    } else if (lastRemoteAlertState == STATE_LOCAL_ALERT_CAUTION) {
                        if (nextState != STATE_REMOTE_ALERT_DANGER) { 
                            nextState = STATE_REMOTE_ALERT_CAUTION;
                        }
                    }

                    if (nextState != STATE_REMOTE_ALERT_DANGER) {
                        if (lastDistRemote <= PRAG_PERICOL_OBSTACOL_REMOTE) {
                            nextState = STATE_REMOTE_ALERT_DANGER;
                        } else if (lastDistRemote <= PRAG_ATENTIE_OBSTACOL_REMOTE) {
                            if (nextState != STATE_REMOTE_ALERT_DANGER) { 
                                nextState = STATE_REMOTE_ALERT_CAUTION;
                            }
                        }
                    }
                  }
                }
                else if (absYaw >= PRAG_CONTRA_SENS) {
                  currentRelPos = DIFFERENT_DIRECTIONS;
                  nextState = STATE_DIFFERENT_DIRECTIONS;
                }

                // --- Bloc de persistență remote (Histerezis pentru alertele remote) ---
                if (g_current_system_state == STATE_REMOTE_ALERT_DANGER || g_current_system_state == STATE_REMOTE_ALERT_CAUTION) {
                    if (nextState == STATE_RUNNING_NORMAL) { 
                        if (g_current_system_state == STATE_REMOTE_ALERT_DANGER) {
                            if (lastDistRemote > PRAG_PERICOL_EXIT_REMOTE && lastRemoteAlertState < STATE_LOCAL_ALERT_DANGER) {
                                nextState = STATE_REMOTE_ALERT_CAUTION; 
                            } else {
                                nextState = STATE_REMOTE_ALERT_DANGER; 
                            }
                        }
                        if (g_current_system_state == STATE_REMOTE_ALERT_CAUTION) {
                            if (lastDistRemote > PRAG_ATENTIE_EXIT_REMOTE && lastRemoteAlertState < STATE_LOCAL_ALERT_CAUTION) {
                                nextState = STATE_RUNNING_NORMAL; 
                            } else {
                                nextState = STATE_REMOTE_ALERT_CAUTION; 
                            }
                        }
                    }
                }
            } else { 
                if (g_current_system_state == STATE_REMOTE_ALERT_DANGER ||
                    g_current_system_state == STATE_REMOTE_ALERT_CAUTION ||
                    g_current_system_state == STATE_DIFFERENT_DIRECTIONS) {
                    nextState = STATE_RUNNING_NORMAL;
                }
            }
        }
    }
  }
  
  // Dacă vehiculul local este în alertă (pericol/atenție), își transmite starea.
  if (nextState == STATE_LOCAL_ALERT_DANGER || nextState == STATE_LOCAL_ALERT_CAUTION) {
    trimiteDate(yawLocal, lastDistLocal, nextState);
  }


  // Verifică dacă starea s-a schimbat
  bool stateChanged = (nextState != g_current_system_state);
  g_current_system_state = nextState;

  // Aplică acțiunile bazate pe starea curentă (Viteza și Buzzer)
  int targetSpeed;

  switch (g_current_system_state) {
    case STATE_WAITING_STATIONARE:
      targetSpeed = VITEZA_STOP;
      break;
    case STATE_LOCAL_ALERT_DANGER:
      targetSpeed = VITEZA_STOP;
      break;
    case STATE_LOCAL_ALERT_CAUTION:
      targetSpeed = VITEZA_INCETINIT;
      break;
    case STATE_REMOTE_ALERT_DANGER:
      targetSpeed = VITEZA_STOP;
      break;
    case STATE_REMOTE_ALERT_CAUTION:
      targetSpeed = VITEZA_INCETINIT;
      break;

    case STATE_DIFFERENT_DIRECTIONS:
    case STATE_RUNNING_NORMAL:
      targetSpeed = VITEZA_NORMAL;
      break;
  }

  setMotorSpeed(targetSpeed);
  setBuzzerState(g_current_system_state); // Transmitem direct SystemState

  // Actualizare afișaj: Doar când starea s-a schimbat sau la un interval fix pentru valori numerice
  // Trimitem viteza filtrata pentru afisaj
  if (stateChanged || (now - lastDisplayUpdateMillis >= DISPLAY_UPDATE_INTERVAL_MS_CONTENT)) {
    afisareStareCurenta(lastYawLocal, hasRemoteData, lastDistLocal, lastDistRemote, g_current_system_state, currentRelPos, g_smoothed_relative_speed_local);
    lastDisplayedState = g_current_system_state;
    lastDisplayedYaw = lastYawLocal;
    lastDisplayedDistLocal = lastDistLocal;
    lastDisplayedDistRemote = lastDistRemote;
    lastDisplayedRelPos = currentRelPos;
    lastDisplayedRelativeSpeedLocal = g_smoothed_relative_speed_local; // Stocam valoarea afisata
    lastDisplayUpdateMillis = now;
  } else {
    // Actualizează doar valorile numerice care se pot schimba constant, fără a redesena tot ecranul
    if (abs(lastYawLocal - lastDisplayedYaw) > 0.5 ||
        abs(lastDistLocal - lastDisplayedDistLocal) > 1.0 ||
        (hasRemoteData && abs(lastDistRemote - lastDisplayedDistRemote) > 1.0) ||
        (currentRelPos != lastDisplayedRelPos) ||
        (abs(g_smoothed_relative_speed_local - lastDisplayedRelativeSpeedLocal) > 0.05) // Prag de 0.05 m/s pentru a redesena (ajustat)
        ) {

        afisareValoriPermanente(lastYawLocal, hasRemoteData, lastDistLocal, lastDistRemote, currentRelPos, g_smoothed_relative_speed_local);
        lastDisplayedYaw = lastYawLocal;
        lastDisplayedDistLocal = lastDistLocal;
        lastDisplayedDistRemote = lastDistRemote;
        lastDisplayedRelPos = currentRelPos;
        lastDisplayedRelativeSpeedLocal = g_smoothed_relative_speed_local;
    }
  }
}