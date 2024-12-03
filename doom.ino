#include "constants.h"
#include "level.h"
#include "sprites.h"
#include "input.h"
#include "entities.h"
#include "types.h"
#include "display.h"
#include "sound.h"

// Macros para operaciones comunes
#define swap(a, b)            do { typeof(a) temp = a; a = b; b = temp; } while (0)
#define sign(a, b)            (double) (a > b ? 1 : (b > a ? -1 : 0))

// Variables globales
uint8_t scene = INTRO;           // Escena actual
bool exit_scene = false;         // Bandera para salir de una escena
bool invert_screen = false;      // Bandera para invertir la pantalla
uint8_t flash_screen = 0;        // Control de parpadeo de pantalla

// Entidades y jugador
Player player;                   // Estructura del jugador
Entity entity[MAX_ENTITIES];     // Array de entidades dinámicas
StaticEntity static_entity[MAX_STATIC_ENTITIES]; // Array de entidades estáticas
uint8_t num_entities = 0;        // Número de entidades dinámicas activas
uint8_t num_static_entities = 0; // Número de entidades estáticas activas

// Configuración inicial del sistema
void setup(void) {
    setupDisplay();    // Configuración de la pantalla
    input_setup();     // Configuración de los controles
    sound_init();      // Inicialización del sistema de sonido
}

// Cambia a una nueva escena
void jumpTo(uint8_t target_scene) {
    scene = target_scene;  // Actualiza la escena
    exit_scene = true;     // Indica que se debe salir de la escena actual
}

// Inicializa el nivel a partir de su definición
void initializeLevel(const uint8_t level[]) {
    for (uint8_t y = LEVEL_HEIGHT - 1; y >= 0; y--) {
        for (uint8_t x = 0; x < LEVEL_WIDTH; x++) {
            uint8_t block = getBlockAt(level, x, y);

            if (block == E_PLAYER) {
                player = create_player(x, y); // Crea el jugador en el nivel
                return;
            }
        }
    }
}

// Obtiene un bloque del nivel según sus coordenadas
uint8_t getBlockAt(const uint8_t level[], uint8_t x, uint8_t y) {
    if (x < 0 || x >= LEVEL_WIDTH || y < 0 || y >= LEVEL_HEIGHT) {
        return E_FLOOR;  // Devuelve un bloque de suelo si las coordenadas son inválidas
    }

    // Obtiene el bloque correcto utilizando la representación comprimida
    return pgm_read_byte(level + (((LEVEL_HEIGHT - 1 - y) * LEVEL_WIDTH + x) / 2))
           >> (!(x % 2) * 4) & 0b1111;
}

// Verifica si una entidad ya está activa
bool isSpawned(UID uid) {
    for (uint8_t i = 0; i < num_entities; i++) {
        if (entity[i].uid == uid) return true;
    }
    return false;
}

// Verifica si una entidad estática ya está activa
bool isStatic(UID uid) {
    for (uint8_t i = 0; i < num_static_entities; i++) {
        if (static_entity[i].uid == uid) return true;
    }
    return false;
}

// Genera una nueva entidad en el mapa
void spawnEntity(uint8_t type, uint8_t x, uint8_t y) {
    if (num_entities >= MAX_ENTITIES) return; // Evita superar el límite

    switch (type) {
        case E_ENEMY:
            entity[num_entities] = create_enemy(x, y); // Crea un enemigo
            num_entities++;
            break;

        case E_KEY:
            entity[num_entities] = create_key(x, y); // Crea una llave
            num_entities++;
            break;

        case E_MEDIKIT:
            entity[num_entities] = create_medikit(x, y); // Crea un botiquín
            num_entities++;
            break;
    }
}

// Genera un proyectil de fuego
void spawnFireball(double x, double y) {
    if (num_entities >= MAX_ENTITIES) return;

    UID uid = create_uid(E_FIREBALL, x, y); // Crea un UID único para el proyectil

    if (isSpawned(uid)) return; // No genera si ya existe

    int16_t dir = FIREBALL_ANGLES + atan2(y - player.pos.y, x - player.pos.x) / PI * FIREBALL_ANGLES;
    if (dir < 0) dir += FIREBALL_ANGLES * 2;
    entity[num_entities] = create_fireball(x, y, dir);
    num_entities++;
}

// Elimina una entidad dinámica
void removeEntity(UID uid, bool makeStatic = false) {
    uint8_t i = 0;
    bool found = false;

    while (i < num_entities) {
        if (!found && entity[i].uid == uid) {
            found = true; // Marca la entidad como encontrada
            num_entities--;
        }

        if (found) {
            entity[i] = entity[i + 1]; // Reorganiza el array
        }

        i++;
    }
}

// Elimina una entidad estática
void removeStaticEntity(UID uid) {
    uint8_t i = 0;
    bool found = false;

    while (i < num_static_entities) {
        if (!found && static_entity[i].uid == uid) {
            found = true; // Marca la entidad como encontrada
            num_static_entities--;
        }

        if (found) {
            static_entity[i] = static_entity[i + 1]; // Reorganiza el array
        }

        i++;
    }
}

// Detecta colisiones con entidades u obstáculos
UID detectCollision(const uint8_t level[], Coords *pos, double relative_x, double relative_y, bool only_walls = false) {
    uint8_t round_x = int(pos->x + relative_x);
    uint8_t round_y = int(pos->y + relative_y);
    uint8_t block = getBlockAt(level, round_x, round_y);

    if (block == E_WALL) {
        playSound(hit_wall_snd, HIT_WALL_SND_LEN);
        return create_uid(block, round_x, round_y);
    }

    if (only_walls) {
        return UID_null;
    }

    for (uint8_t i = 0; i < num_entities; i++) {
        if (&(entity[i].pos) == pos) {
            continue;
        }

        uint8_t type = uid_get_type(entity[i].uid);

        if (type != E_ENEMY || entity[i].state == S_DEAD || entity[i].state == S_HIDDEN) {
            continue;
        }

        Coords new_coords = { entity[i].pos.x - relative_x, entity[i].pos.y - relative_y };
        uint8_t distance = coords_distance(pos, &new_coords);

        if (distance < ENEMY_COLLIDER_DIST && distance < entity[i].distance) {
            return entity[i].uid;
        }
    }

    return UID_null;
}

// Dispara un arma
void fire() {
    playSound(shoot_snd, SHOOT_SND_LEN); // Reproduce el sonido del disparo

    for (uint8_t i = 0; i < num_entities; i++) {
        if (uid_get_type(entity[i].uid) != E_ENEMY || entity[i].state == S_DEAD || entity[i].state == S_HIDDEN) {
            continue; // Ignora entidades no válidas
        }

        Coords transform = translateIntoView(&(entity[i].pos)); // Traduce las coordenadas al espacio de visión
        if (abs(transform.x) < 20 && transform.y > 0) {
            uint8_t damage = (double) min(GUN_MAX_DAMAGE, GUN_MAX_DAMAGE / (abs(transform.x) * entity[i].distance) / 5);
            if (damage > 0) {
                entity[i].health = max(0, entity[i].health - damage); // Reduce la salud de la entidad
                entity[i].state = S_HIT; // Cambia el estado de la entidad
                entity[i].timer = 4; // Establece un temporizador para el estado
            }
        }
    }
}

// Actualiza la posición de una entidad y detecta colisiones
UID updatePosition(const uint8_t level[], Coords *pos, double relative_x, double relative_y, bool only_walls = false) {
    UID collide_x = detectCollision(level, pos, relative_x, 0, only_walls); // Detecta colisión en X
    UID collide_y = detectCollision(level, pos, 0, relative_y, only_walls); // Detecta colisión en Y

    if (!collide_x) pos->x += relative_x; // Actualiza X si no hay colisión
    if (!collide_y) pos->y += relative_y; // Actualiza Y si no hay colisión

    return collide_x || collide_y || UID_null; // Retorna el UID de la colisión si ocurre
}

// Actualiza el estado de todas las entidades activas
void updateEntities(const uint8_t level[]) {
    uint8_t i = 0;
    while (i < num_entities) {
        entity[i].distance = coords_distance(&(player.pos), &(entity[i].pos)); // Calcula la distancia al jugador

        if (entity[i].timer > 0) entity[i].timer--; // Decrementa el temporizador si está activo

        if (entity[i].distance > MAX_ENTITY_DISTANCE) {
            removeEntity(entity[i].uid); // Elimina entidades fuera del rango
            continue;
        }

        if (entity[i].state == S_HIDDEN) {
            i++;
            continue;
        }

        uint8_t type = uid_get_type(entity[i].uid);
        switch (type) {
            case E_ENEMY: {
                // Gestión de enemigos
                if (entity[i].health == 0) {
                    if (entity[i].state != S_DEAD) {
                        entity[i].state = S_DEAD;
                        entity[i].timer = 6;
                    }
                } else if (entity[i].state == S_HIT) {
                    if (entity[i].timer == 0) {
                        entity[i].state = S_ALERT;
                        entity[i].timer = 40;
                    }
                } else if (entity[i].state == S_FIRING) {
                    if (entity[i].timer == 0) {
                        entity[i].state = S_ALERT;
                        entity[i].timer = 40;
                    }
                } else {
                    // Movimiento y acciones de enemigos
                    if (entity[i].distance > ENEMY_MELEE_DIST && entity[i].distance < MAX_ENEMY_VIEW) {
                        if (entity[i].state != S_ALERT) {
                            entity[i].state = S_ALERT;
                            entity[i].timer = 20;
                        } else if (entity[i].timer == 0) {
                            spawnFireball(entity[i].pos.x, entity[i].pos.y); // Lanza un proyectil
                            entity[i].state = S_FIRING;
                            entity[i].timer = 6;
                        } else {
                            updatePosition(
                                level,
                                &(entity[i].pos),
                                sign(player.pos.x, entity[i].pos.x) * ENEMY_SPEED * delta,
                                sign(player.pos.y, entity[i].pos.y) * ENEMY_SPEED * delta,
                                true
                            );
                        }
                    } else if (entity[i].distance <= ENEMY_MELEE_DIST) {
                        if (entity[i].state != S_MELEE) {
                            entity[i].state = S_MELEE;
                            entity[i].timer = 10;
                        } else if (entity[i].timer == 0) {
                            player.health = max(0, player.health - ENEMY_MELEE_DAMAGE); // Reduce la salud del jugador
                            entity[i].timer = 14;
                            flash_screen = 1;
                            updateHud();
                        }
                    } else {
                        entity[i].state = S_STAND; // El enemigo se detiene
                    }
                }
                break;
            }
            case E_FIREBALL: {
                // Gestión de proyectiles
                if (entity[i].distance < FIREBALL_COLLIDER_DIST) {
                    player.health = max(0, player.health - ENEMY_FIREBALL_DAMAGE); // Reduce la salud del jugador
                    flash_screen = 1;
                    updateHud();
                    removeEntity(entity[i].uid);
                    continue;
                } else {
                    UID collided = updatePosition(
                        level,
                        &(entity[i].pos),
                        cos((double) entity[i].health / FIREBALL_ANGLES * PI) * FIREBALL_SPEED,
                        sin((double) entity[i].health / FIREBALL_ANGLES * PI) * FIREBALL_SPEED,
                        true
                    );

                    if (collided) {
                        removeEntity(entity[i].uid);
                        continue;
                    }
                }
                break;
            }
            case E_MEDIKIT: {
                // Gestión de botiquines
                if (entity[i].distance < ITEM_COLLIDER_DIST) {
                    playSound(medkit_snd, MEDKIT_SND_LEN);
                    entity[i].state = S_HIDDEN;
                    player.health = min(100, player.health + 50); // Restaura la salud del jugador
                    updateHud();
                    flash_screen = 1;
                }
                break;
            }
            case E_KEY: {
                // Gestión de llaves
                if (entity[i].distance < ITEM_COLLIDER_DIST) {
                    playSound(get_key_snd, GET_KEY_SND_LEN);
                    entity[i].state = S_HIDDEN;
                    player.keys++; // Incrementa el contador de llaves del jugador
                    updateHud();
                    flash_screen = 1;
                }
                break;
            }
        }
        i++;
    }
}

// Renderiza el mapa, trazando rayos desde el jugador
void renderMap(const uint8_t level[], double view_height) {
    UID last_uid;

    for (uint8_t x = 0; x < SCREEN_WIDTH; x += RES_DIVIDER) {
        double camera_x = 2 * (double)x / SCREEN_WIDTH - 1;
        double ray_x = player.dir.x + player.plane.x * camera_x;
        double ray_y = player.dir.y + player.plane.y * camera_x;
        uint8_t map_x = uint8_t(player.pos.x);
        uint8_t map_y = uint8_t(player.pos.y);
        Coords map_coords = {player.pos.x, player.pos.y};
        double delta_x = abs(1 / ray_x);
        double delta_y = abs(1 / ray_y);

        int8_t step_x;
        int8_t step_y;
        double side_x;
        double side_y;

        if (ray_x < 0) {
            step_x = -1;
            side_x = (player.pos.x - map_x) * delta_x;
        } else {
            step_x = 1;
            side_x = (map_x + 1.0 - player.pos.x) * delta_x;
        }

        if (ray_y < 0) {
            step_y = -1;
            side_y = (player.pos.y - map_y) * delta_y;
        } else {
            step_y = 1;
            side_y = (map_y + 1.0 - player.pos.y) * delta_y;
        }

        uint8_t depth = 0;
        bool hit = 0;
        bool side;

        while (!hit && depth < MAX_RENDER_DEPTH) {
            if (side_x < side_y) {
                side_x += delta_x;
                map_x += step_x;
                side = 0;
            } else {
                side_y += delta_y;
                map_y += step_y;
                side = 1;
            }

            uint8_t block = getBlockAt(level, map_x, map_y);

            if (block == E_WALL) {
                hit = 1;
            } else {
                if (block == E_ENEMY || (block & 0b00001000)) {
                    if (coords_distance(&(player.pos), &map_coords) < MAX_ENTITY_DISTANCE) {
                        UID uid = create_uid(block, map_x, map_y);
                        if (last_uid != uid && !isSpawned(uid)) {
                            spawnEntity(block, map_x, map_y);
                            last_uid = uid;
                        }
                    }
                }
            }

            depth++;
        }

        if (hit) {
            double distance;

            if (side == 0) {
                distance = max(1, (map_x - player.pos.x + (1 - step_x) / 2) / ray_x);
            } else {
                distance = max(1, (map_y - player.pos.y + (1 - step_y) / 2) / ray_y);
            }

            zbuffer[x / Z_RES_DIVIDER] = min(distance * DISTANCE_MULTIPLIER, 255);

            uint8_t line_height = RENDER_HEIGHT / distance;

            drawVLine(
                x,
                view_height / distance - line_height / 2 + RENDER_HEIGHT / 2,
                view_height / distance + line_height / 2 + RENDER_HEIGHT / 2,
                GRADIENT_COUNT - int(distance / MAX_RENDER_DEPTH * GRADIENT_COUNT) - side * 2
            );
        }
    }
}

// Ordena las entidades según su distancia al jugador para el renderizado
uint8_t sortEntities() {
    uint8_t gap = num_entities;
    bool swapped = false;

    while (gap > 1 || swapped) {
        gap = (gap * 10) / 13;
        if (gap == 9 || gap == 10) gap = 11;
        if (gap < 1) gap = 1;

        swapped = false;

        for (uint8_t i = 0; i < num_entities - gap; i++) {
            uint8_t j = i + gap;

            if (entity[i].distance < entity[j].distance) {
                swap(entity[i], entity[j]);
                swapped = true;
            }
        }
    }
}

// Traduce las coordenadas de una entidad al espacio de visión del jugador
Coords translateIntoView(Coords *pos) {
    double sprite_x = pos->x - player.pos.x;
    double sprite_y = pos->y - player.pos.y;

    double inv_det = 1.0 / (player.plane.x * player.dir.y - player.dir.x * player.plane.y);
    double transform_x = inv_det * (player.dir.y * sprite_x - player.dir.x * sprite_y);
    double transform_y = inv_det * (-player.plane.y * sprite_x + player.plane.x * sprite_y);

    return {transform_x, transform_y};
}

// Renderiza todas las entidades en pantalla
void renderEntities(double view_height) {
    sortEntities();

    for (uint8_t i = 0; i < num_entities; i++) {
        if (entity[i].state == S_HIDDEN) continue;

        Coords transform = translateIntoView(&(entity[i].pos));

        if (transform.y <= 0.1 || transform.y > MAX_SPRITE_DEPTH) continue;

        int16_t sprite_screen_x = HALF_WIDTH * (1.0 + transform.x / transform.y);
        int8_t sprite_screen_y = RENDER_HEIGHT / 2 + view_height / transform.y;
        uint8_t type = uid_get_type(entity[i].uid);

        if (sprite_screen_x < -HALF_WIDTH || sprite_screen_x > SCREEN_WIDTH + HALF_WIDTH) continue;

        switch (type) {
            case E_ENEMY: {
                uint8_t sprite;
                if (entity[i].state == S_ALERT) {
                    sprite = int(millis() / 500) % 2;
                } else if (entity[i].state == S_FIRING) {
                    sprite = 2;
                } else if (entity[i].state == S_HIT) {
                    sprite = 3;
                } else if (entity[i].state == S_MELEE) {
                    sprite = entity[i].timer > 10 ? 2 : 1;
                } else if (entity[i].state == S_DEAD) {
                    sprite = entity[i].timer > 0 ? 3 : 4;
                } else {
                    sprite = 0;
                }

                drawSprite(
                    sprite_screen_x - BMP_IMP_WIDTH * .5 / transform.y,
                    sprite_screen_y - 8 / transform.y,
                    bmp_imp_bits,
                    bmp_imp_mask,
                    BMP_IMP_WIDTH,
                    BMP_IMP_HEIGHT,
                    sprite,
                    transform.y
                );
                break;
            }
            case E_FIREBALL: {
                drawSprite(
                    sprite_screen_x - BMP_FIREBALL_WIDTH / 2 / transform.y,
                    sprite_screen_y - BMP_FIREBALL_HEIGHT / 2 / transform.y,
                    bmp_fireball_bits,
                    bmp_fireball_mask,
                    BMP_FIREBALL_WIDTH,
                    BMP_FIREBALL_HEIGHT,
                    0,
                    transform.y
                );
                break;
            }
            case E_MEDIKIT: {
                drawSprite(
                    sprite_screen_x - BMP_ITEMS_WIDTH / 2 / transform.y,
                    sprite_screen_y + 5 / transform.y,
                    bmp_items_bits,
                    bmp_items_mask,
                    BMP_ITEMS_WIDTH,
                    BMP_ITEMS_HEIGHT,
                    0,
                    transform.y
                );
                break;
            }
            case E_KEY: {
                drawSprite(
                    sprite_screen_x - BMP_ITEMS_WIDTH / 2 / transform.y,
                    sprite_screen_y + 5 / transform.y,
                    bmp_items_bits,
                    bmp_items_mask,
                    BMP_ITEMS_WIDTH,
                    BMP_ITEMS_HEIGHT,
                    1,
                    transform.y
                );
                break;
            }
        }
    }
}

// Renderiza el arma en la pantalla
void renderGun(uint8_t gun_pos, double amount_jogging) {
    char x = 48 + sin((double)millis() * JOGGING_SPEED) * 10 * amount_jogging;
    char y = RENDER_HEIGHT - gun_pos + abs(cos((double)millis() * JOGGING_SPEED)) * 8 * amount_jogging;

    if (gun_pos > GUN_SHOT_POS - 2) {
        display.drawBitmap(x + 6, y - 11, bmp_fire_bits, BMP_FIRE_WIDTH, BMP_FIRE_HEIGHT, 1);
    }

    uint8_t clip_height = max(0, min(y + BMP_GUN_HEIGHT, RENDER_HEIGHT) - y);

    display.drawBitmap(x, y, bmp_gun_mask, BMP_GUN_WIDTH, clip_height, 0);
    display.drawBitmap(x, y, bmp_gun_bits, BMP_GUN_WIDTH, clip_height, 1);
}
// Renderiza el HUD (Head-Up Display) en la pantalla
void renderHud() {
    drawText(2, 58, F("{}"), 0);         // Muestra la salud del jugador
    drawText(40, 58, F("[]"), 0);        // Muestra el número de llaves
    updateHud();                         // Actualiza el HUD con los valores actuales
}

// Actualiza la información mostrada en el HUD
void updateHud() {
    display.clearRect(12, 58, 15, 6);    // Limpia el área de la salud
    display.clearRect(50, 58, 5, 6);     // Limpia el área de las llaves

    drawText(12, 58, player.health);     // Dibuja la salud del jugador
    drawText(50, 58, player.keys);       // Dibuja la cantidad de llaves del jugador
}

// Renderiza las estadísticas adicionales en pantalla (FPS y número de entidades)
void renderStats() {
    display.clearRect(58, 58, 70, 6);    // Limpia el área de estadísticas
    drawText(114, 58, int(getActualFps())); // Muestra los FPS actuales
    drawText(82, 58, num_entities);      // Muestra el número de entidades activas
}

// Lógica del bucle para la escena de introducción
void loopIntro() {
    display.drawBitmap(
        (SCREEN_WIDTH - BMP_LOGO_WIDTH) / 2,
        (SCREEN_HEIGHT - BMP_LOGO_HEIGHT) / 3,
        bmp_logo_bits,
        BMP_LOGO_WIDTH,
        BMP_LOGO_HEIGHT,
        1
    );

    delay(1000); // Pausa para mostrar el logo
    drawText(SCREEN_WIDTH / 2 - 25, SCREEN_HEIGHT * .8, F("PRESS FIRE")); // Instrucción para continuar
    display.display();

    while (!exit_scene) {
#ifdef SNES_CONTROLLER
        getControllerData();
#endif
        if (input_fire()) jumpTo(GAME_PLAY); // Cambia a la escena de juego si se presiona disparar
    };
}

// Lógica del bucle para la escena de juego
void loopGamePlay() {
    bool gun_fired = false;          // Estado del disparo
    bool walkSoundToggle = false;    // Alterna entre sonidos de caminar
    uint8_t gun_pos = 0;             // Posición del arma
    double rot_speed;                // Velocidad de rotación
    double old_dir_x;                // Dirección anterior en X
    double old_plane_x;              // Dirección del plano anterior
    double view_height;              // Altura de la vista
    double jogging;                  // Intensidad del movimiento
    uint8_t fade = GRADIENT_COUNT - 1; // Gradiente de desvanecimiento

    initializeLevel(sto_level_1);    // Inicializa el primer nivel

    do {
        fps();                       // Calcula los FPS actuales

        memset(display_buf, 0, SCREEN_WIDTH * (RENDER_HEIGHT / 8)); // Limpia el búfer de la pantalla

#ifdef SNES_CONTROLLER
        getControllerData();         // Obtiene datos del controlador si está habilitado
#endif

        if (player.health > 0) {
            if (input_up()) {
                player.velocity += (MOV_SPEED - player.velocity) * .4; // Aumenta la velocidad hacia adelante
                jogging = abs(player.velocity) * MOV_SPEED_INV;       // Calcula el movimiento
            } else if (input_down()) {
                player.velocity += (-MOV_SPEED - player.velocity) * .4; // Aumenta la velocidad hacia atrás
                jogging = abs(player.velocity) * MOV_SPEED_INV;
            } else {
                player.velocity *= .5;         // Reduce la velocidad gradualmente
                jogging = abs(player.velocity) * MOV_SPEED_INV;
            }

            if (input_right()) {
                rot_speed = ROT_SPEED * delta; // Calcula la rotación
                old_dir_x = player.dir.x;
                player.dir.x = player.dir.x * cos(-rot_speed) - player.dir.y * sin(-rot_speed);
                player.dir.y = old_dir_x * sin(-rot_speed) + player.dir.y * cos(-rot_speed);
                old_plane_x = player.plane.x;
                player.plane.x = player.plane.x * cos(-rot_speed) - player.plane.y * sin(-rot_speed);
                player.plane.y = old_plane_x * sin(-rot_speed) + player.plane.y * cos(-rot_speed);
            } else if (input_left()) {
                rot_speed = ROT_SPEED * delta;
                old_dir_x = player.dir.x;
                player.dir.x = player.dir.x * cos(rot_speed) - player.dir.y * sin(rot_speed);
                player.dir.y = old_dir_x * sin(rot_speed) + player.dir.y * cos(rot_speed);
                old_plane_x = player.plane.x;
                player.plane.x = player.plane.x * cos(rot_speed) - player.plane.y * sin(rot_speed);
                player.plane.y = old_plane_x * sin(rot_speed) + player.plane.y * cos(rot_speed);
            }

            view_height = abs(sin((double)millis() * JOGGING_SPEED)) * 6 * jogging; // Ajusta la altura de la vista

            if (view_height > 5.9) {
                if (!walkSoundToggle) {
                    playSound(walk1_snd, WALK1_SND_LEN);
                    walkSoundToggle = true;
                } else {
                    playSound(walk2_snd, WALK2_SND_LEN);
                    walkSoundToggle = false;
                }
            }

            if (gun_pos > GUN_TARGET_POS) {
                gun_pos -= 1; // Retroceso del arma
            } else if (gun_pos < GUN_TARGET_POS) {
                gun_pos += 2;
            } else if (!gun_fired && input_fire()) {
                gun_pos = GUN_SHOT_POS; // Disparo del arma
                gun_fired = true;
                fire();
            } else if (gun_fired && !input_fire()) {
                gun_fired = false; // Estado de no disparo
            }
        } else {
            if (view_height > -10) view_height--; // Ajusta la vista al morir
            else if (input_fire()) jumpTo(INTRO); // Reinicia al intro

            if (gun_pos > 1) gun_pos -= 2;
        }

        if (abs(player.velocity) > 0.003) {
            updatePosition(
                sto_level_1,
                &(player.pos),
                player.dir.x * player.velocity * delta,
                player.dir.y * player.velocity * delta
            );
        } else {
            player.velocity = 0;
        }

        updateEntities(sto_level_1);      // Actualiza las entidades
        renderMap(sto_level_1, view_height); // Renderiza el mapa
        renderEntities(view_height);      // Renderiza las entidades
        renderGun(gun_pos, jogging);      // Renderiza el arma

        if (fade > 0) {
            fadeScreen(fade);             // Realiza el efecto de desvanecimiento
            fade--;

            if (fade == 0) {
                renderHud();              // Muestra el HUD al terminar el desvanecimiento
            }
        } else {
            renderStats();                // Muestra estadísticas en pantalla
        }

        if (flash_screen > 0) {
            invert_screen = !invert_screen; // Invierte la pantalla temporalmente
            flash_screen--;
        } else if (invert_screen) {
            invert_screen = 0;            // Vuelve a la normalidad
        }

        display.invertDisplay(invert_screen);
        display.display();

#ifdef SNES_CONTROLLER
        if (input_start()) {
#else
        if (input_left() && input_right()) {
#endif
            jumpTo(INTRO); // Vuelve al intro si se presionan los botones de salir
        }
    } while (!exit_scene);
}

// Bucle principal del programa
void loop(void) {
    switch (scene) {
        case INTRO: {
            loopIntro(); // Escena de introducción
            break;
        }
        case GAME_PLAY: {
            loopGamePlay(); // Escena de juego
            break;
        }
    }

    for (uint8_t i = 0; i < GRADIENT_COUNT; i++) {
        fadeScreen(i, 0); // Realiza el efecto de desvanecimiento
        display.display();
        delay(40);
    }
    exit_scene = false;
}

