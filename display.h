#include <stdint.h>

/* Declare bitmap array containing font */
uint8_t font[128 * 8];
uint8_t displayBuff[128 * 4];

/* Declare text buffer for display output */
extern char textbuffer[4][16];

void display_init();

void display_string(int line, char *s);

void display_buffer(int x, int size, uint8_t *data);

void display_rect(int x, int y, int size);

void display_debug(volatile int *const addr);

void display_clear(void);

void display_update();

void enable_interrupt(void);

static void num32asc(char *s, int n);

char *itoaconv(int num);

void quicksleep(int cyc);

void draw_border(void);