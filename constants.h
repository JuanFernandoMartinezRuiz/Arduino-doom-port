#ifndef _constants_h
#define _constants_h

// ================================================
// CONFIGURACIÓN DE CONSTANTES Y DEFINICIONES
// ================================================

// ------------------------------------
// Configuración de los pines para controles
// ------------------------------------

// Uso del modo PULLUP en las entradas de los controles
#define USE_INPUT_PULLUP

// Pines para controles básicos
#define K_LEFT              6       // Pin para el botón "Izquierda"
#define K_RIGHT             7       // Pin para el botón "Derecha"
#define K_UP                8       // Pin para el botón "Arriba"
#define K_DOWN              3       // Pin para el botón "Abajo"
#define K_FIRE              10      // Pin para el botón "Disparar"

// ------------------------------------
// Controlador SNES (opcional)
// ------------------------------------

// Descomenta la siguiente línea para habilitar soporte para controlador SNES
// #define SNES_CONTROLLER

// Pines para la comunicación con el controlador SNES
constexpr uint8_t DATA_CLOCK   = 11;    // Pin de reloj
constexpr uint8_t DATA_LATCH   = 12;    // Pin de latch
constexpr uint8_t DATA_SERIAL  = 13;    // Pin de datos seriales

// ------------------------------------
// Configuración de sonido
// ------------------------------------

// Pin para la salida de sonido (no cambiar, depende del temporizador usado)
constexpr uint8_t SOUND_PIN   = 9;

// ------------------------------------
// Configuración gráfica
// ------------------------------------

// Optimización para pantallas SSD1306
#define OPTIMIZE_SSD1306

// Tiempo deseado por cuadro en ms (~15 FPS)
#define FRAME_TIME          66.666666  

// Configuración de resolución
#define RES_DIVIDER         2           // Divide la resolución horizontal; valores más altos reducen el uso de memoria y proceso
#define Z_RES_DIVIDER       2           // Divide la resolución del Z-buffer; sacrifica resolución para ahorrar memoria

// Configuración de precisión de distancia
#define DISTANCE_MULTIPLIER 20          // Multiplicador para aumentar la precisión de las distancias en uint8_t
#define MAX_RENDER_DEPTH    12          // Profundidad máxima para el raycasting
#define MAX_SPRITE_DEPTH    8           // Profundidad máxima para sprites renderizados

// Tamaño del Z-buffer
#define ZBUFFER_SIZE        SCREEN_WIDTH / Z_RES_DIVIDER

// ------------------------------------
// Configuración del nivel
// ------------------------------------

// Ancho base del nivel (en bits)
#define LEVEL_WIDTH_BASE    6
#define LEVEL_WIDTH         (1 << LEVEL_WIDTH_BASE)  // Calcula el ancho del nivel en píxeles
#define LEVEL_HEIGHT        57                      // Altura del nivel en píxeles
#define LEVEL_SIZE          LEVEL_WIDTH / 2 * LEVEL_HEIGHT // Tamaño del nivel comprimido

// ------------------------------------
// Escenas
// ------------------------------------
#define INTRO                 0       // Escena de introducción
#define GAME_PLAY             1       // Escena principal del juego

// ------------------------------------
// Configuración del juego
// ------------------------------------

// Posición inicial del arma
#define GUN_TARGET_POS        18      
#define GUN_SHOT_POS          GUN_TARGET_POS + 4  // Posición al disparar

// Velocidades
#define ROT_SPEED             .12     // Velocidad de rotación del jugador
#define MOV_SPEED             .2      // Velocidad de movimiento del jugador
#define MOV_SPEED_INV         5       // Inverso de la velocidad de movimiento

#define JOGGING_SPEED         .005    // Velocidad de oscilación al caminar
#define ENEMY_SPEED           .02     // Velocidad de movimiento de enemigos
#define FIREBALL_SPEED        .2      // Velocidad de proyectiles
#define FIREBALL_ANGLES       45      // Número de ángulos por PI para proyectiles

// Límite de entidades activas
#define MAX_ENTITIES          10      // Número máximo de entidades activas
#define MAX_STATIC_ENTITIES   28      // Número máximo de entidades estáticas

// Distancias máximas y rangos de colisión
#define MAX_ENTITY_DISTANCE   200     // Distancia máxima para entidades activas (* DISTANCE_MULTIPLIER)
#define MAX_ENEMY_VIEW        80      // Distancia máxima de visión del enemigo (* DISTANCE_MULTIPLIER)
#define ITEM_COLLIDER_DIST    6       // Distancia para recoger ítems (* DISTANCE_MULTIPLIER)
#define ENEMY_COLLIDER_DIST   4       // Distancia para colisiones con enemigos (* DISTANCE_MULTIPLIER)
#define FIREBALL_COLLIDER_DIST 2      // Distancia de colisión para proyectiles (* DISTANCE_MULTIPLIER)
#define ENEMY_MELEE_DIST      6       // Distancia para ataques cuerpo a cuerpo (* DISTANCE_MULTIPLIER)
#define WALL_COLLIDER_DIST    .2      // Distancia de colisión con paredes

// Daños
#define ENEMY_MELEE_DAMAGE    8       // Daño de enemigos en cuerpo a cuerpo
#define ENEMY_FIREBALL_DAMAGE 20      // Daño de proyectiles enemigos
#define GUN_MAX_DAMAGE        15      // Daño máximo del arma del jugador

// ------------------------------------
// Configuración de la pantalla
// ------------------------------------

// Dimensiones de la pantalla
constexpr uint8_t SCREEN_WIDTH     = 128; // Ancho en píxeles
constexpr uint8_t SCREEN_HEIGHT    = 64;  // Altura en píxeles
constexpr uint8_t HALF_WIDTH       = SCREEN_WIDTH / 2; // Mitad del ancho
constexpr uint8_t RENDER_HEIGHT    = 56;  // Altura utilizada para renderizar (el resto es HUD)
constexpr uint8_t HALF_HEIGHT      = SCREEN_HEIGHT / 2; // Mitad de la altura

#endif
