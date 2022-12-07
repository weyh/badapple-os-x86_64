#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "gtdio.h"

#define DEFAULT_COLOR (PRINT_COLOR_LIGHT_GRAY | PRINT_COLOR_BLACK << 4)

typedef struct {
    uint8_t c;
    uint8_t color;
} vid_char;

#define NUM_COLS ((uint8_t)80)
#define NUM_ROWS ((uint8_t)25)

static vid_char *const vid_buffer = (vid_char *)0xB8000;
static uint8_t ccol = 0, crow = 0;
static uint8_t color = DEFAULT_COLOR;

static void clear_row(uint8_t row) {
    vid_char blank = {' ', color};

    for (uint8_t col = 0; col < NUM_COLS; col++) {
        vid_buffer[row * NUM_COLS + col] = blank;
    }
}

void clear() {
    for (uint8_t row = 0; row < NUM_ROWS; row++) {
        clear_row(row);
    }
}

static void print_newline() {
    ccol = 0;
    if (crow < NUM_ROWS - 1) {
        crow++;
        return;
    }

    for (uint8_t row = 1; row < NUM_ROWS; row++) {
        for (uint8_t col = 0; col < NUM_COLS; col++) {
            vid_buffer[(row - 1) * NUM_COLS + col] =
                vid_buffer[row * NUM_COLS + col];
        }
    }

    clear_row(NUM_ROWS - 1);
}

void putc(char c) {
    if (c == '\n') {
        print_newline();
        return;
    }

    if (c == '\r') {
        set_cursor_col(0);
        return;
    }

    if (ccol > NUM_COLS) {
        print_newline();
    }

    vid_buffer[crow * NUM_COLS + ccol] = (vid_char){(uint8_t)c, color};
    ccol++;
}

void puts(const char *str) {
    while (*str) {
        putc(*str++);
    }
}

void printf_unsigned(unsigned long long number, int radix) {
    static const char hexChars[] = "0123456789abcdef";

    char buffer[32];
    int pos = 0;

    // convert number to ASCII
    do {
        unsigned long long rem = number % radix;
        number /= radix;
        buffer[pos++] = hexChars[rem];
    } while (number > 0);

    // print number in reverse order
    while (--pos >= 0)
        putc(buffer[pos]);
}

void printf_signed(long long number, int radix) {
    if (number < 0) {
        putc('-');
        printf_unsigned(-number, radix);
    } else
        printf_unsigned(number, radix);
}

#define PRINTF_STATE_NORMAL 0
#define PRINTF_STATE_LENGTH 1
#define PRINTF_STATE_LENGTH_SHORT 2
#define PRINTF_STATE_LENGTH_LONG 3
#define PRINTF_STATE_SPEC 4

#define PRINTF_LENGTH_DEFAULT 0
#define PRINTF_LENGTH_SHORT_SHORT 1
#define PRINTF_LENGTH_SHORT 2
#define PRINTF_LENGTH_LONG 3
#define PRINTF_LENGTH_LONG_LONG 4

void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int state = PRINTF_STATE_NORMAL;
    int length = PRINTF_LENGTH_DEFAULT;

    int radix = 10;
    bool sign = false;
    bool number = false;

    while (*fmt) {
        switch (state) {
        case PRINTF_STATE_NORMAL:
            if (*fmt == '%')
                state = PRINTF_STATE_LENGTH;
            else
                putc(*fmt);
            break;
        case PRINTF_STATE_LENGTH:
            if (*fmt == 'h') {
                length = PRINTF_LENGTH_SHORT;
                state = PRINTF_STATE_LENGTH_SHORT;
            } else if (*fmt == 'l') {
                length = PRINTF_LENGTH_LONG;
                state = PRINTF_STATE_LENGTH_LONG;
            } else
                goto PRINTF_STATE_SPEC_;

            break;
        case PRINTF_STATE_LENGTH_SHORT:
            if (*fmt == 'h') {
                length = PRINTF_LENGTH_SHORT_SHORT;
                state = PRINTF_STATE_SPEC;
            } else
                goto PRINTF_STATE_SPEC_;
            break;
        case PRINTF_STATE_LENGTH_LONG:
            if (*fmt == 'l') {
                length = PRINTF_LENGTH_LONG_LONG;
                state = PRINTF_STATE_SPEC;
            } else
                goto PRINTF_STATE_SPEC_;
            break;
        case PRINTF_STATE_SPEC:
        PRINTF_STATE_SPEC_:
            switch (*fmt) {
            case 'c':
                putc((char)va_arg(args, int));
                break;
            case 's':
                puts(va_arg(args, const char *));
                break;
            case '%':
                putc('%');
                break;
            case 'd':
            case 'i':
                radix = 10;
                sign = true;
                number = true;
                break;
            case 'u':
                radix = 10;
                sign = false;
                number = true;
                break;
            case 'X':
            case 'x':
            case 'p':
                radix = 16;
                sign = false;
                number = true;
                break;
            case 'o':
                radix = 8;
                sign = false;
                number = true;
                break;
            // ignore invalid spec
            default:
                break;
            }

            if (number) {
                if (sign) {
                    switch (length) {
                    case PRINTF_LENGTH_SHORT_SHORT:
                    case PRINTF_LENGTH_SHORT:
                    case PRINTF_LENGTH_DEFAULT:
                        printf_signed(va_arg(args, int), radix);
                        break;

                    case PRINTF_LENGTH_LONG:
                        printf_signed(va_arg(args, long), radix);
                        break;

                    case PRINTF_LENGTH_LONG_LONG:
                        printf_signed(va_arg(args, long long), radix);
                        break;
                    }
                } else {
                    switch (length) {
                    case PRINTF_LENGTH_SHORT_SHORT:
                    case PRINTF_LENGTH_SHORT:
                    case PRINTF_LENGTH_DEFAULT:
                        printf_unsigned(va_arg(args, unsigned int), radix);
                        break;

                    case PRINTF_LENGTH_LONG:
                        printf_unsigned(va_arg(args, unsigned long), radix);
                        break;

                    case PRINTF_LENGTH_LONG_LONG:
                        printf_unsigned(va_arg(args, unsigned long long),
                                        radix);
                        break;
                    }
                }
            }

            // reset state
            state = PRINTF_STATE_NORMAL;
            length = PRINTF_LENGTH_DEFAULT;
            radix = 10;
            sign = false;
            number = false;
            break;
        }

        fmt++;
    }

    va_end(args);
}

void set_color(uint8_t fg, uint8_t bg) { color = fg | bg << 4; }

void reset_color() { color = DEFAULT_COLOR; }

void set_cursor_row(uint8_t row) {
    if (row >= NUM_ROWS)
        return;
    crow = row;
}

void set_cursor_col(uint8_t col) {
    if (col >= NUM_COLS)
        return;
    ccol = col;
}
