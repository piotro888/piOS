#include "tty.h"
#include <driver/vga.h>
#include <libk/types.h>
#include <libk/ringbuff.h>
#include <libk/con/semaphore.h>
#include <libk/con/spinlock.h>
#include <libk/math.h>
#include <libk/assert.h>
#include <fs/vfs.h>
#include <proc/sched.h>
#include <sys/sysres.h>

static int tty_cursor_col, tty_cursor_row;

#define TTY_BUFF_SIZE 255
#define TTY_SUBMIT_BUFF 64
#define LINE_BUFF_SIZE 128
static unsigned char line_buff[LINE_BUFF_SIZE];
static int line_buff_len;

static struct ringbuff read_rb;
static struct semaphore read_data_signal;
static struct spinlock read_sl;

static struct ringbuff write_rb;
static struct semaphore write_data_signal, write_not_full;
static struct spinlock write_sl;

static int read_notify = 0, write_notify = 0;

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
    if(read_notify) {
        read_notify = 0;
        sysres_notify();
    }
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

        // NOTE: Reversing line wrapping on backspace is not compliant with posix tty
        // posix does only '\b' ' ' '\b' which stops on begin of screen line
        vga_put_char_at(' ', tty_cursor_row, tty_cursor_col);
    }
}

void linebuff_putc(char c) {
    if(line_buff_len < TTY_BUFF_SIZE)
        line_buff[line_buff_len++] = c;
}

// TODO: Implement reprint (see stty rprnt) on ctrl-r? & VKILL
// FIXME: FLUSH TTY ON panic
/* NOTE: Keyboard arrows are not handled in canon mode tty in linux and cursor l/r don't change  overwrite
 * position in line buffer. Maybe we could implement this in some mode (move cursor and lb pos)? */

void tty_submit_char(char c) {
    if(IS_PRINTABLE(c)) {
        // save in line buffer ~ ICANON mode, only on input from kbd
        linebuff_putc(c);
        // echo char in console
        tty_putc(c);
    } else {
        /* Some characters have different meaning when entered from keyboard */
        switch (c) {
            // in linux ctrl-v to skip interpretation and pass to `default` handling
            case '\n':
                linebuff_putc(c);
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
                linebuff_putc(c);
                /* In Linux tty, until -ctlecho is set all chars are displayed in caret notation here,
                 * if -ctlecho is disabled then it passes control escapes to tetminal putc and interprets them there,
                 * allowing to send escapes from keyboard */
        }
    }
}

/* Hacky escape code parsing */
static int esc_stage = 0;
static int esc_args[2];
static int in_escape_code = 0;
void handle_escapes(char c) {
    if(esc_stage == 0) {
        if(c != '[') {
            in_escape_code = 0;
            esc_stage = 0;
            return; // bad code
        }

        esc_stage++;
        esc_args[0] = esc_args[1] = 0;
        return;
    }

    if(esc_stage == 1 || esc_stage == 2) { // parsing arguments
        if(c == ';')
            esc_stage++;
        else if(c >= '0' && c <= '9') {
            esc_args[esc_stage-1] *= 10;
            esc_args[esc_stage-1] += c-'0';
        } else {
            esc_stage = 3;
        }
    }

    if(esc_stage == 3) { // final byte
        if(c == 'm') { // set color
            int fg = (esc_args[0] < 38 ? esc_args[0]-30 : (esc_args[0]-90)|(0x08));
            int bg = (esc_args[1] < 38 ? (esc_args[1] ? esc_args[1]-30 : 0) : (esc_args[1]-90)|(0x08))<<4;
            vga_set_color(fg | bg);
        } else if(c == 'A') {
            tty_cursor_row -= MAX(MAX(esc_args[0], 1), 0);
        } else if(c == 'B') {
            for(int i=0; i<MAX(esc_args[0], 1); i++)
                tty_new_line();
        } else if(c == 'C') {
            tty_cursor_col = MIN(tty_cursor_col+MAX(esc_args[0], 1), VGA_TEXT_WIDTH-1);
        } else if(c == 'D') {
            tty_cursor_col = MAX(tty_cursor_col-MAX(esc_args[0], 1), 0);
        } else if(c == 'H') {
            tty_cursor_col = MAX(esc_args[0], VGA_TEXT_WIDTH);
            tty_cursor_row = MAX(esc_args[1], VGA_TEXT_HEIGHT);
        }

        in_escape_code = 0;
        esc_stage = 0;
    }
}

void tty_putc(char c) {
    if(in_escape_code) {
        handle_escapes(c);
        return;
    }

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
            case '\033': // ESC char (this is ^[)
                in_escape_code = 1;
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

int tty_fs_get_fid(char* path) { (void)path; return 0; }

ssize_t tty_fs_read(struct fd_info* file, void* buff, size_t len) {
    (void) file;
    spinlock_lock(&read_sl);
    while(ringbuff_length(&read_rb) < 1) {
        spinlock_unlock(&read_sl);
        semaphore_down(&read_data_signal);
        spinlock_lock(&read_sl);
    }

    size_t rl = ringbuff_read(&read_rb, buff, len);

    if(ringbuff_length(&read_rb)) {
        semaphore_binary_up(&read_data_signal);
        if(read_notify)
            sysres_notify();
    }

    spinlock_unlock(&read_sl);
    return rl;
}

ssize_t tty_fs_read_nonblock(struct fd_info* file, void* buff, size_t len) {
    (void) file;
    spinlock_lock(&read_sl);
    if(ringbuff_length(&read_rb) < 1) {
        read_notify = 1; // FIXME_LATER: set if !ioctl
        spinlock_unlock(&read_sl);
        return -EWOULDBLOCK;
    }
    if(!len)
        return 0;

    size_t rl = ringbuff_read(&read_rb, buff, len);

    if(ringbuff_length(&read_rb)) {
        semaphore_binary_up(&read_data_signal);
        if(read_notify) {
            read_notify = 0;
            sysres_notify();
        }
    }
    spinlock_unlock(&read_sl);
    return rl;
}

ssize_t tty_fs_write(struct fd_info* file, void* buff, size_t len) {
    (void) file;
    spinlock_lock(&write_sl);
    size_t write_len = ringbuff_write(&write_rb, buff, len);
    while (write_len < len) {
        semaphore_down(&write_not_full);
        write_len += ringbuff_write(&write_rb, (u8*)buff+write_len, len-write_len);
    }

    spinlock_unlock(&write_sl);
    semaphore_binary_up(&write_data_signal);
    return write_len;
}

ssize_t tty_fs_write_nonblock(struct fd_info* file, void* buff, size_t len) {
    (void) file;
    spinlock_lock(&write_sl);
    if(ringbuff_length(&write_rb)+len > TTY_BUFF_SIZE) {
        write_notify = 1;
        spinlock_unlock(&write_sl);
        return -EWOULDBLOCK;
    }
    size_t write_len = ringbuff_write(&write_rb, buff, len);
    ASSERT(write_len == len);

    spinlock_unlock(&write_sl);
    semaphore_binary_up(&write_data_signal);
    return write_len;
}

void __attribute__((noreturn)) tty_driver_thread() {
    /* submits tty input in order from fs buffer */
    unsigned char buff[TTY_SUBMIT_BUFF];
    for (;;) {
        // no spinlock is needed - only this thread is reading from ringbuff
        while (!ringbuff_length(&write_rb))
            semaphore_down(&write_data_signal);

        size_t read_len = ringbuff_read(&write_rb, buff, TTY_SUBMIT_BUFF);
        semaphore_binary_up(&write_not_full);
        if(write_notify) {
            write_notify = 0;
            sysres_notify();
        }

        for(size_t i=0; i<read_len; i++)
            tty_putc(buff[i]);
    }
}

void tty_direct_write(char* buff, size_t len) {
    tty_fs_write(NULL, buff, len);
}

void tty_mnt_vfs() {
    const struct vfs_reg reg = {
            tty_fs_get_fid,
            tty_fs_read,
            tty_fs_write,
            tty_fs_read_nonblock,
            tty_fs_write_nonblock,
    };
    vfs_mount("/dev/tty", &reg);
}

// Initialize basic TTY/VGA functionality to use kprintf during boot (no malloc allowed here!)
void tty_init_basic() {
    vga_init(); // init gpu to text mode

    tty_cursor_col = tty_cursor_row = 0;
    line_buff_len = 0;

    vga_clear_screen();
}

// Initialize TTY driver over FS device. Requires tty_init_basic to be called beforehand
void tty_init_driver() {
    ringbuff_init(&read_rb, TTY_BUFF_SIZE);
    semaphore_init(&read_data_signal);
    spinlock_init(&read_sl);
    ringbuff_init(&write_rb, TTY_BUFF_SIZE);
    semaphore_init(&write_data_signal);
    semaphore_init(&write_not_full);
    spinlock_init(&write_sl);
}

void tty_register_thread() {
    make_kernel_thread("drv_tty", tty_driver_thread);
}
