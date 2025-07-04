// ===== Fișier: config.h =====
#ifndef CONFIG_H
#define CONFIG_H

// Pini Arduino UNO
#define PIN_BT_RX          3
#define PIN_BT_TX          2
#define PIN_TFT_CS         10
#define PIN_TFT_DC         9
#define PIN_TFT_RST        8
#define PIN_TRIG           7
#define PIN_ECHO           6
#define PIN_MOTOR_PWM      5
#define PIN_MOTOR_DIR_A    4

#define PIN_BUZZER         A0 // Pinul pentru buzzer

// Parametri giroscop
#define GYRO_CALIB_SAMPLES 20
#define GYRO_SENSITIVITY   131.0f
#define SMOOTHING_ALPHA    0.02f
#define YAW_RATE_THRESHOLD 0.5f

// Parametri distanță
#define DISTANCE_MARGIN_V2V 5.0f

// Praguri temporale si de distanta
#define INTERVAL_MS                         200UL
#define WAIT_START_MS                       10000UL
#define DISPLAY_UPDATE_INTERVAL_MS_CONTENT  2000UL

// NOU: Timeout pentru datele remote (după cât timp o alertă remote expiră dacă nu mai primești date)
#define REMOTE_DATA_TIMEOUT_MS 700UL // *** ESENȚIAL: ASIGURĂ-TE CĂ ACEST ESTE PREZENT ȘI NU COMENTAT ***

// Praguri de distanta si unghi
#define PRAG_ACEEASI_DIRECTIE_STRICT    5.0f
#define PRAG_ACEEASI_DIRECTIE_LARG      45.0f
#define PRAG_CONTRA_SENS                135.0f

// NOU: Pragurile de distanta locala cu Histerezis
#define HYSTERESIS_MARGIN_CM    5.0f

// Praguri de intrare/ieșire din stări, bazate pe histerezis
#define PRAG_PERICOL_ENTRY_LOCAL    25.0f
#define PRAG_PERICOL_EXIT_LOCAL     (PRAG_PERICOL_ENTRY_LOCAL + HYSTERESIS_MARGIN_CM)

#define PRAG_ATENTIE_ENTRY_MIN_LOCAL    (PRAG_PERICOL_EXIT_LOCAL)
#define PRAG_ATENTIE_ENTRY_MAX_LOCAL    45.0f
#define PRAG_ATENTIE_EXIT_MIN_LOCAL     (PRAG_ATENTIE_ENTRY_MIN_LOCAL - HYSTERESIS_MARGIN_CM)
#define PRAG_ATENTIE_EXIT_MAX_LOCAL     (PRAG_ATENTIE_ENTRY_MAX_LOCAL + HYSTERESIS_MARGIN_CM)

// Pragurile remote rămân la fel
#define PRAG_ATENTIE_OBSTACOL_REMOTE    40.0f
#define PRAG_PERICOL_OBSTACOL_REMOTE    20.0f
// Hysteresis pentru alertele remote
#define PRAG_PERICOL_EXIT_REMOTE    (PRAG_PERICOL_OBSTACOL_REMOTE + HYSTERESIS_MARGIN_CM)
#define PRAG_ATENTIE_EXIT_REMOTE    (PRAG_ATENTIE_OBSTACOL_REMOTE + HYSTERESIS_MARGIN_CM)

// Viteze motoare
#define VITEZA_STOP         0
#define VITEZA_INCETINIT    180
#define VITEZA_NORMAL       100

// Parametri pentru calculul vitezei relative (rate of change of distance)
#define MIN_DIST_CHANGE_FOR_MOVEMENT 2.0f // cm/interval - schimbare minima pentru a considera miscare (ex: 2cm in 200ms)
#define MIN_TIME_FOR_VELOCITY_CALC   50UL // ms - minim timp necesar intre masuratori pentru calculul vitezei

// NOU: Praguri de histerezis pentru viteza de apropiere (pentru a inhiba/permite alertele)
#define SPEED_HYSTERESIS_MARGIN_MPS 0.05f // m/s - marja de histerezis pentru viteza
#define MIN_APPROACH_SPEED_FOR_ALERT_ENTRY 0.2f // m/s - viteza minima de apropiere pentru a permite o alerta
#define MIN_APPROACH_SPEED_FOR_ALERT_EXIT  (MIN_APPROACH_SPEED_FOR_ALERT_ENTRY - SPEED_HYSTERESIS_MARGIN_MPS) // m/s - viteza sub care se iese din conditia de "viteza suficienta"

// Factor de netezire pentru viteza relativa
#define SPEED_SMOOTHING_ALPHA 0.3f // 0.0 (nu filtreaza) la 1.0 (filtrare maxima, reactioneaza lent)


// Definirea stărilor sistemului
enum SystemState {
  STATE_WAITING_STATIONARE,
  STATE_LOCAL_ALERT_DANGER,
  STATE_LOCAL_ALERT_CAUTION,
  STATE_REMOTE_ALERT_DANGER,
  STATE_REMOTE_ALERT_CAUTION,
  STATE_DIFFERENT_DIRECTIONS,
  STATE_RUNNING_NORMAL
};

// Definirea pozitiei relative pentru afisaj
enum RelativePosition {
  UNKNOWN,
  IN_FRONT_PARTNER,
  BEHIND_PARTNER,
  DIFFERENT_DIRECTIONS
};

// Declarare externa a starii curente a sistemului (definita in .ino)
extern SystemState g_current_system_state;
extern int g_current_motor_speed;

#endif // CONFIG_H