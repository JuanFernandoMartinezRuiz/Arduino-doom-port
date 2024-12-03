/* 
 * Archivo: Display.h
 * Propósito: Gestión de la pantalla OLED SSD1306 y renderizado de gráficos.
 * Nota: Mover este código a CPP parece aumentar el uso de memoria Flash. Revisar por qué.
 */

#include "SSD1306.h"
#include "constants.h"

// Macro para leer un carácter de una cadena en memoria Flash
#define F_char(ifsh, ch)    pgm_read_byte(reinterpret_cast<PGM_P>(ifsh) + ch)

// Máscaras para leer bits de izquierda a derecha, optimizando bitRead
const static uint8_t PROGMEM bit_mask[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };
#define read_bit(b, n)      b & pgm_read_byte(bit_mask + n) ? 1 : 0

// Declaración de funciones
void setupDisplay();
void fps();
bool getGradientPixel(uint8_t x, uint8_t y, uint8_t i);
void fadeScreen(uint8_t intensity, bool color);
void drawByte(uint8_t x, uint8_t y, uint8_t b);
uint8_t getByte(uint8_t x, uint8_t y);
void drawPixel(int8_t x, int8_t y, bool color, bool raycasterViewport);
void drawVLine(uint8_t x, int8_t start_y, int8_t end_y, uint8_t intensity);
void drawSprite(int8_t x, int8_t y, const uint8_t bitmap[], const uint8_t mask[], int16_t w, int16_t h, uint8_t sprite, double distance);
void drawChar(int8_t x, int8_t y, char ch);
void drawText(int8_t x, int8_t y, char *txt, uint8_t space = 1);
void drawText(int8_t x, int8_t y, const __FlashStringHelper *txt, uint8_t space = 1);

// Inicialización de la pantalla OLED
Adafruit_SSD1306<SCREEN_WIDTH, SCREEN_HEIGHT> display;

// Control de FPS (fotogramas por segundo)
double delta = 1;              // Variación de tiempo entre fotogramas
uint32_t lastFrameTime = 0;    // Tiempo del último fotograma

#ifdef OPTIMIZE_SSD1306
uint8_t *display_buf;          // Buffer directo para optimización de SSD1306
#endif

uint8_t zbuffer[ZBUFFER_SIZE]; // Buffer de profundidad para raycasting

// ------------------------------------
// Configuración de la pantalla
// ------------------------------------
void setupDisplay() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("Error al inicializar SSD1306"));
        while (1); // Detiene la ejecución si falla la inicialización
    }

#ifdef OPTIMIZE_SSD1306
    display_buf = display.getBuffer();
#endif

    memset(zbuffer, 0xFF, ZBUFFER_SIZE); // Inicializa el z-buffer
}

// Control de FPS: limita la velocidad de actualización y calcula delta
void fps() {
    while (millis() - lastFrameTime < FRAME_TIME); // Espera el tiempo necesario
    delta = (double)(millis() - lastFrameTime) / FRAME_TIME; // Calcula delta
    lastFrameTime = millis();
}

// Devuelve los FPS actuales
double getActualFps() {
    return 1000 / (FRAME_TIME * delta);
}

// ------------------------------------
// Dibujado y renderizado
// ------------------------------------

// Dibuja un byte verticalmente en el buffer de pantalla
void drawByte(uint8_t x, uint8_t y, uint8_t b) {
#ifdef OPTIMIZE_SSD1306
    display_buf[(y / 8) * SCREEN_WIDTH + x] = b;
#endif
}

// Obtiene un píxel de un gradiente predefinido
boolean getGradientPixel(uint8_t x, uint8_t y, uint8_t i) {
    if (i == 0) return 0; // Sin gradiente
    if (i >= GRADIENT_COUNT - 1) return 1; // Gradiente completo

    uint8_t index = max(0, min(GRADIENT_COUNT - 1, i)) * GRADIENT_WIDTH * GRADIENT_HEIGHT
                    + y * GRADIENT_WIDTH % (GRADIENT_WIDTH * GRADIENT_HEIGHT)
                    + x / GRADIENT_HEIGHT % GRADIENT_WIDTH;
    return read_bit(pgm_read_byte(gradient + index), x % 8);
}

// Efecto de desvanecimiento de pantalla
void fadeScreen(uint8_t intensity, bool color = 0) {
    for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {
        for (uint8_t y = 0; y < SCREEN_HEIGHT; y++) {
            if (getGradientPixel(x, y, intensity))
                drawPixel(x, y, color, false);
        }
    }
}

// Dibujado optimizado de píxeles
void drawPixel(int8_t x, int8_t y, bool color, bool raycasterViewport = false) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= (raycasterViewport ? RENDER_HEIGHT : SCREEN_HEIGHT)) return;

#ifdef OPTIMIZE_SSD1306
    if (color) {
        display_buf[x + (y / 8) * SCREEN_WIDTH] |= (1 << (y & 7));
    } else {
        display_buf[x + (y / 8) * SCREEN_WIDTH] &= ~(1 << (y & 7));
    }
#else
    display.drawPixel(x, y, color);
#endif
}

// Dibuja una línea vertical con gradiente
void drawVLine(uint8_t x, int8_t start_y, int8_t end_y, uint8_t intensity) {
    int8_t lower_y = max(min(start_y, end_y), 0);
    int8_t higher_y = min(max(start_y, end_y), RENDER_HEIGHT - 1);

#ifdef OPTIMIZE_SSD1306
    uint8_t b = 0, bp;
    for (uint8_t c = 0; c < RES_DIVIDER; c++) {
        for (int8_t y = lower_y; y <= higher_y; y++) {
            bp = y % 8;
            b |= getGradientPixel(x + c, y, intensity) << bp;
            if (bp == 7) {
                drawByte(x + c, y, b);
                b = 0;
            }
        }
        if (bp != 7) drawByte(x + c, higher_y, b);
    }
#else
    for (int8_t y = lower_y; y <= higher_y; y++) {
        for (uint8_t c = 0; c < RES_DIVIDER; c++) {
            if (getGradientPixel(x + c, y, intensity)) drawPixel(x + c, y, 1, true);
        }
    }
#endif
}

// Renderiza un sprite con escala, máscara, y corrección de perspectiva
void drawSprite(
    int8_t x, int8_t y,
    const uint8_t bitmap[], const uint8_t mask[],
    int16_t w, int16_t h,
    uint8_t sprite, double distance
) {
    // Control de profundidad basado en z-buffer
    if (zbuffer[min(max(x, 0), ZBUFFER_SIZE - 1) / Z_RES_DIVIDER] < distance * DISTANCE_MULTIPLIER) {
        return;
    }

    uint8_t tw = w / distance;
    uint8_t th = h / distance;
    uint8_t pixel_size = max(1, 1.0 / distance);

    for (uint8_t ty = 0; ty < th; ty += pixel_size) {
        if (y + ty < 0 || y + ty >= RENDER_HEIGHT) continue;

        for (uint8_t tx = 0; tx < tw; tx += pixel_size) {
            if (x + tx < 0 || x + tx >= SCREEN_WIDTH) continue;

            uint8_t sx = tx * distance;
            uint8_t sy = ty * distance;
            uint8_t byte_offset = sprite * (w / 8) * h + sy * (w / 8) + sx / 8;

            if (read_bit(pgm_read_byte(mask + byte_offset), sx % 8)) {
                bool pixel = read_bit(pgm_read_byte(bitmap + byte_offset), sx % 8);
                for (uint8_t ox = 0; ox < pixel_size; ox++) {
                    for (uint8_t oy = 0; oy < pixel_size; oy++) {
                        drawPixel(x + tx + ox, y + ty + oy, pixel, true);
                    }
                }
            }
        }
    }
}

// Renderiza un carácter individual en pantalla
void drawChar(int8_t x, int8_t y, char ch) {
    uint8_t c = 0;
    while (CHAR_MAP[c] != ch && CHAR_MAP[c] != '\0') c++;
    uint8_t bOffset = c / 2;

    for (uint8_t line = 0; line < CHAR_HEIGHT; line++) {
        uint8_t b = pgm_read_byte(bmp_font + (line * bmp_font_width + bOffset));
        for (uint8_t n = 0; n < CHAR_WIDTH; n++) {
            if (read_bit(b, (c % 2 == 0 ? 0 : 4) + n)) drawPixel(x + n, y + line, 1, false);
        }
    }
}

// Renderiza una cadena de texto en pantalla
void drawText(int8_t x, int8_t y, char *txt, uint8_t space) {
    uint8_t pos = x;
    uint8_t i = 0;
    char ch;
    while ((ch = txt[i]) != '\0') {
        drawChar(pos, y, ch);
        i++;
        pos += CHAR_WIDTH + space;
        if (pos > SCREEN_WIDTH) return;
    }
}

// Renderiza una cadena almacenada en memoria Flash
void drawText(int8_t x, int8_t y, const __FlashStringHelper *txt_p, uint8_t space) {
    uint8_t pos = x;
    uint8_t i = 0;
    char ch;
    while ((ch = F_char(txt_p, i)) != '\0') {
        drawChar(pos, y, ch);
        i++;
        pos += CHAR_WIDTH + space;
    }
}

// Renderiza un número en pantalla (máximo 3 dígitos)
void drawText(uint8_t x, uint8_t y, uint8_t num) {
    char buf[4];
    itoa(num, buf, 10);
    drawText(x, y, buf);
}
