#ifndef _entities_h
#define _entities_h

#include "types.h"

// ================================================
// CONFIGURACIÓN DE ENTIDADES Y ATAJOS
// ================================================

// ------------------------------------
// Atajos para crear diferentes tipos de entidades
// ------------------------------------

// Crea un jugador con posición inicial, dirección y plano de cámara.
#define create_player(x, y)   { \
    create_coords((double) x + 0.5, (double) y + 0.5), /* Posición inicial ajustada */ \
    create_coords(1, 0),                               /* Dirección inicial (hacia la derecha) */ \
    create_coords(0, -0.66),                           /* Plano de cámara para proyección 2D */ \
    0,                                                 /* Velocidad inicial */ \
    100,                                               /* Salud inicial */  \
  }

// Crea un enemigo con salud inicial y estado predeterminado.
#define create_enemy(x, y)            create_entity(E_ENEMY, x, y, S_STAND, 100)

// Crea un botiquín sin salud (ítem recolectable).
#define create_medikit(x, y)          create_entity(E_MEDIKIT, x, y, S_STAND, 0)

// Crea una llave como ítem recolectable.
#define create_key(x, y)              create_entity(E_KEY, x, y, S_STAND, 0)

// Crea un proyectil (bola de fuego) con dirección inicial.
#define create_fireball(x, y, dir)    create_entity(E_FIREBALL, x, y, S_STAND, dir)

// ------------------------------------
// Estados de las entidades
// ------------------------------------
#define S_STAND               0  // La entidad está quieta
#define S_ALERT               1  // La entidad está alerta
#define S_FIRING              2  // La entidad está disparando
#define S_MELEE               3  // La entidad está en combate cuerpo a cuerpo
#define S_HIT                 4  // La entidad ha sido golpeada
#define S_DEAD                5  // La entidad está muerta
#define S_HIDDEN              6  // La entidad está oculta
#define S_OPEN                7  // La entidad está abierta (por ejemplo, una puerta)
#define S_CLOSE               8  // La entidad está cerrada (por ejemplo, una puerta)

// ------------------------------------
// Estructuras de datos para entidades
// ------------------------------------

// Estructura del jugador
struct Player { 
  Coords pos;        // Posición del jugador
  Coords dir;        // Dirección del jugador
  Coords plane;      // Plano de la cámara del jugador
  double velocity;   // Velocidad actual
  uint8_t health;    // Salud del jugador
  uint8_t keys;      // Número de llaves recolectadas
};

// Estructura de una entidad dinámica
struct Entity {
  UID uid;           // Identificador único de la entidad
  Coords pos;        // Posición de la entidad
  uint8_t state;     // Estado actual de la entidad
  uint8_t health;    // Salud de la entidad (ángulo en el caso de proyectiles)
  uint8_t distance;  // Distancia al jugador (usado para ordenamiento)
  uint8_t timer;     // Temporizador para cambios de estado
};

// Estructura de una entidad estática
struct StaticEntity { 
  UID uid;           // Identificador único de la entidad
  uint8_t x;         // Coordenada X
  uint8_t y;         // Coordenada Y
  bool active;       // Indica si la entidad está activa o no
};

// ------------------------------------
// Declaraciones de funciones
// ------------------------------------

// Crea una entidad dinámica en el juego.
Entity create_entity(uint8_t type, uint8_t x, uint8_t y, uint8_t initialState, uint8_t initialHealth);

// Crea una entidad estática en el juego.
StaticEntity create_static_entity(UID uid, uint8_t x, uint8_t y, bool active);

#endif
