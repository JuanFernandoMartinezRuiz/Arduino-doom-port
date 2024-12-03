#include <Arduino.h>
#include "input.h"
#include "constants.h"

// ================================================
// CONFIGURACIÓN DE ENTRADAS
// ================================================

// Configuración para usar entradas con o sin resistencia PULLUP
#ifdef USE_INPUT_PULLUP
  #define INPUT_MODE INPUT_PULLUP  // Modo de entrada con resistencia pull-up
  #define INPUT_STATE LOW          // Estado activo cuando el pin está en LOW
#else
  #define INPUT_MODE INPUT         // Modo de entrada sin resistencia pull-up
  #define INPUT_STATE HIGH         // Estado activo cuando el pin está en HIGH
#endif

// ------------------------------------
// Configuración para controlador SNES
// ------------------------------------
#ifdef SNES_CONTROLLER
uint16_t buttons = 0; // Variable para almacenar el estado de los botones

/**
 * Configura los pines necesarios para el controlador SNES.
 */
void input_setup() {
  pinMode(DATA_CLOCK, OUTPUT);        // Pin de reloj configurado como salida
  digitalWrite(DATA_CLOCK, HIGH);    // Inicialmente en HIGH

  pinMode(DATA_LATCH, OUTPUT);       // Pin de latch configurado como salida
  digitalWrite(DATA_LATCH, LOW);     // Inicialmente en LOW

  pinMode(DATA_SERIAL, OUTPUT);      // Pin de datos configurado como salida
  digitalWrite(DATA_SERIAL, HIGH);   // Inicialmente en HIGH
  pinMode(DATA_SERIAL, INPUT);       // Cambia el pin de datos a entrada
}

/**
 * Obtiene el estado de los botones del controlador SNES.
 * Los datos se obtienen del registro de desplazamiento usando el reloj.
 */
void getControllerData(void) {
  digitalWrite(DATA_LATCH, HIGH);    // Activa el latch por 12us
  delayMicroseconds(12);
  digitalWrite(DATA_LATCH, LOW);     // Desactiva el latch
  delayMicroseconds(6);
  buttons = 0;                       // Resetea el estado de los botones

  // Lee el estado de los 16 botones
  for (uint8_t i = 0; i < 16; ++i) {
    digitalWrite(DATA_CLOCK, LOW);   // Baja el reloj
    delayMicroseconds(6);
    buttons |= !digitalRead(DATA_SERIAL) << i; // Almacena el estado del botón
    digitalWrite(DATA_CLOCK, HIGH);  // Sube el reloj
    delayMicroseconds(6);
  }
}

// Funciones para detectar el estado de botones específicos
bool input_left() { return buttons & LEFT; }
bool input_right() { return buttons & RIGHT; }
bool input_up() { return buttons & UP; }
bool input_down() { return buttons & DOWN; }
bool input_fire() { return buttons & Y; }
bool input_start() { return buttons & START; }

#else // ------------------------------------
// Configuración para botones individuales
// ------------------------------------

/**
 * Configura los pines para botones físicos individuales.
 */
void input_setup() {
  pinMode(K_LEFT, INPUT_MODE);  // Configura el pin para "Izquierda"
  pinMode(K_RIGHT, INPUT_MODE); // Configura el pin para "Derecha"
  pinMode(K_UP, INPUT_MODE);    // Configura el pin para "Arriba"
  pinMode(K_DOWN, INPUT_MODE);  // Configura el pin para "Abajo"
  pinMode(K_FIRE, INPUT_MODE);  // Configura el pin para "Disparar"
}

// Funciones para detectar el estado de cada botón
bool input_left() { return digitalRead(K_LEFT) == INPUT_STATE; }
bool input_right() { return digitalRead(K_RIGHT) == INPUT_STATE; }
bool input_up() { return digitalRead(K_UP) == INPUT_STATE; }
bool input_down() { return digitalRead(K_DOWN) == INPUT_STATE; }
bool input_fire() { return digitalRead(K_FIRE) == INPUT_STATE; }

#endif
