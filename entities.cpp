#include <stdint.h>
#include "entities.h"
#include "types.h"
#include "constants.h"

/**
 * Crea una entidad dinámica en el juego.
 * 
 * @param type Tipo de la entidad (por ejemplo, enemigo, ítem).
 * @param x Coordenada X en el nivel.
 * @param y Coordenada Y en el nivel.
 * @param initialState Estado inicial de la entidad.
 * @param initialHealth Salud inicial de la entidad.
 * @return Una nueva instancia de `Entity` inicializada con los valores dados.
 */
Entity create_entity(uint8_t type, uint8_t x, uint8_t y, uint8_t initialState, uint8_t initialHealth) {
    UID uid = create_uid(type, x, y); // Crea un UID único para la entidad
    Coords pos = create_coords((double)x + .5, (double)y + .5); // Calcula las coordenadas iniciales
    Entity new_entity = { uid, pos, initialState, initialHealth, 0, 0 }; // Inicializa la entidad
    return new_entity;
}

/**
 * Crea una entidad estática en el juego.
 * 
 * @param uid UID único de la entidad.
 * @param x Coordenada X en el nivel.
 * @param y Coordenada Y en el nivel.
 * @param active Indica si la entidad está activa o inactiva.
 * @return Una nueva instancia de `StaticEntity` inicializada con los valores dados.
 */
StaticEntity crate_static_entity(UID uid, uint8_t x, uint8_t y, bool active) {
    return { uid, x, y, active }; // Inicializa y retorna la entidad estática
}
