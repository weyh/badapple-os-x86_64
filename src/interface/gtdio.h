#ifndef __PRINT_H__
#define __PRINT_H__

#include <stdint.h>
#include <stddef.h>

#define PRINT_COLOR_BLACK 0
#define PRINT_COLOR_BLUE 1
#define PRINT_COLOR_GREEN 2
#define PRINT_COLOR_CYAN 3
#define PRINT_COLOR_RED 4
#define PRINT_COLOR_MAGENTA 5
#define PRINT_COLOR_BROWN 6
#define PRINT_COLOR_LIGHT_GRAY 7
#define PRINT_COLOR_DARK_GRAY 8
#define PRINT_COLOR_LIGHT_BLUE 9
#define PRINT_COLOR_LIGHT_GREEN 10
#define PRINT_COLOR_LIGHT_CYAN 11
#define PRINT_COLOR_LIGHT_RED 12
#define PRINT_COLOR_PINK 13
#define PRINT_COLOR_YELLOW 14
#define PRINT_COLOR_WHITE 15

void clear();

void putc(char);
void puts(const char *);

void printf(const char *fmt, ...);

void set_color(uint8_t fg, uint8_t bg);
void reset_color();

void set_cursor_row(uint8_t);
void set_cursor_col(uint8_t);

#define set_cursor(row, col)                                                   \
    do {                                                                       \
        set_cursor_row(row);                                                   \
        set_cursor_col(col);                                                   \
    } while (0)

#endif
