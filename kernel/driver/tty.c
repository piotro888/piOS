#include "tty.h"
#include <driver/vga.h>
#include <libk/types.h>
#include <libk/ringbuff.h>
#include <libk/con/semaphore.h>

static int tty_cursor_col, tty_cursor_row;

#define TTY_BUFF_SIZE 128
static char line_buff[TTY_BUFF_SIZE];
static int line_buff_len;

static struct ringbuff read_rb;
static struct semaphore read_data_signal;

#define IS_PRINTABLE(x) ((x) >= 32 && (x) <= 126)

void tty_new_line() {
    tty_cursor_row++;
    if(tty_cursor_row >= VGA_TEXT_HEIGHT) {
        vga_fast_scroll();
        tty_cursor_row = VGA_TEXT_HEIGHT-1;
    }
}

void tty_submit_line_buff() {
    ringbuff_write(&read_rb, line_buff, line_buff_len);
    line_buff_len = 0;
    semaphore_binary_up(&read_data_signal);
}

void tty_linebuff_erase() {
    if (line_buff_len > 0) {
        line_buff_len--;

        if (tty_cursor_col > 0) {
            tty_cursor_col--;
        } else {
            if (tty_cursor_row > 0)
                tty_cursor_row--;
            tty_cursor_col = VGA_TEXT_WIDTH;
        }

        // NOTE: Reversing line wrapping on backspace is not compatible with posix
        // posix does only '\b' ' ' '\b' which stops on begin of screen
        vga_put_char_at(' ', tty_cursor_row, tty_cursor_col);
    }
}

// TODO: Implement reprint (see stty rprnt) on ctrl-r? & VKILL
/* NOTE: Keyboard arrows are not handled in canon mode tty in linux and cursor l/r don't change  overwrite
 * position in line buffer. Maybe we could implement this in some mode (move cursor and lb pos)? */

void tty_submit_char(char c) {
    if(IS_PRINTABLE(c)) {
        // save in line buffer ~ ICANON mode, only on input from kbd
        if(line_buff_len < TTY_BUFF_SIZE)
            line_buff[line_buff_len++] = c;

        // echo char in console
        tty_putc(c);
    } else {
        /* Some characters have different meaning when entered from keyboard */
        switch (c) {
            case '\n':
                tty_submit_line_buff();
                tty_new_line();
                tty_cursor_col = 0;
                break;
            case '\b':
                tty_linebuff_erase();
                break;
            default:
                // print non-printable in caret notation (^@+code)
                tty_putc('^');
                if(c <= 31)
                    tty_putc('@' + c);
                else
                    tty_putc('?'); // DEL (127) character
        }
    }
}

void tty_putc(char c) {
    if(!IS_PRINTABLE(c)) {
        switch(c) {
            case '\n':
                tty_new_line();
                tty_cursor_col = 0;
                break;
            case '\b':
                if(tty_cursor_col > 0)
                    tty_cursor_col--;
                break;
            default:
                break;
        }
        return;
    }

    vga_put_char_at(c, tty_cursor_row, tty_cursor_col);

    tty_cursor_col++;
    if(tty_cursor_col >= VGA_TEXT_WIDTH) {
        tty_cursor_col = 0;
        tty_new_line();
    }
}

/* Set tty color for new text 4 msb-bg color 4 lsb-fg color */
void tty_set_color(uint8_t color) {
    vga_set_color(color);
}

void tty_puts(const char* str) {
    while(*str)
        tty_putc(*str++);
}

void init_tty() {
    vga_init(); // init gpu to text mode

    tty_cursor_col = tty_cursor_row = 0;
    line_buff_len = 0;
    ringbuff_init(&read_rb, TTY_BUFF_SIZE);
    semaphore_init(&read_data_signal);

    vga_clear_screen();
}
