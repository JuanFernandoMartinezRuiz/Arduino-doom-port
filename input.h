#ifndef _input_h
#define _input_h

/**
 * Enumera los botones disponibles en el controlador SNES y su representación en bits.
 * Cada botón tiene un valor único para facilitar la detección mediante operaciones bitwise.
 */
enum BUTTONS {
  B = 0x0001,        // Botón B
  Y = 0x0002,        // Botón Y
  SELECT = 0x0004,   // Botón Select
  START = 0x0008,    // Botón Start
  UP = 0x0010,       // Dirección Arriba
  DOWN = 0x0020,     // Dirección Abajo
  LEFT = 0x0040,     // Dirección Izquierda
  RIGHT = 0x0080,    // Dirección Derecha
  A = 0x0100,        // Botón A
  X = 0x0200,        // Botón X
  LB = 0x0400,       // Botón L (Shoulder Button Izquierdo)
  RB = 0x0800        // Botón R (Shoulder Button Derecho)
};

// ------------------------------------
// Declaración de funciones
// ------------------------------------

/**
 * Configura los pines necesarios para el controlador o botones individuales.
 */
void input_setup();

/**
 * Verifica si la dirección "Arriba" está activa.
 * @return `true` si el botón de "Arriba" está presionado, `false` en caso contrario.
 */
bool input_up();

/**
 * Verifica si la dirección "Abajo" está activa.
 * @return `true` si el botón de "Abajo" está presionado, `false` en caso contrario.
 */
bool input_down();

/**
 * Verifica si la dirección "Izquierda" está activa.
 * @return `true` si el botón de "Izquierda" está presionado, `false` en caso contrario.
 */
bool input_left();

/**
 * Verifica si la dirección "Derecha" está activa.
 * @return `true` si el botón de "Derecha" está presionado, `false` en caso contrario.
 */
bool input_right();

/**
 * Verifica si el botón de "Disparo" está activo.
 * @return `true` si el botón de disparo (por ejemplo, "Y") está presionado, `false` en caso contrario.
 */
bool input_fire();

#ifdef SNES_CONTROLLER
/**
 * Verifica si el botón "Start" está activo (solo para SNES Controller).
 * @return `true` si el botón "Start" está presionado, `false` en caso contrario.
 */
bool input_start();

/**
 * Obtiene los datos del controlador SNES.
 * Lee el estado de los botones a través del registro de desplazamiento.
 */
void getControllerData(void);
#endif

#endif
