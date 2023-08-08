#include "tty.h"
#include <driver/vga.h>
#include <libk/types.h>
#include <libk/ringbuff.h>
#include <libk/kmalloc.h>
#include <libk/con/semaphore.h>
#include <libk/con/spinlock.h>
#include <libk/math.h>
#include <libk/assert.h>
#include <fs/vfs.h>
#include <proc/sched.h>\

static int tty_cursor_col, tty_cursor_row;

#define TTY_BUFF_SIZE 255
#define TTY_SUBMIT_BUFF 128
#define LINE_BUFF_SIZE 128
static unsigned char line_buff[LINE_BUFF_SIZE];
static int line_buff_len;

static struct ringbuff read_rb;

static struct semaphore list_read_sema, read_data_signal;
static struct list req_read_list;
static struct semaphore list_write_sema, insert_lock;
static struct list req_write_list;

static size_t read_queue_buff = 0;

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

void __attribute__((noreturn)) tty_driver_thread_write() {
    for (;;) {
        semaphore_down(&list_write_sema);
        struct vfs_async_req_t* req = req_write_list.first->val;

        req->size = MIN(req->size, TTY_SUBMIT_BUFF);
        for (int i=0; i<req->size; i++)
            tty_putc(((char*)req->vbuff)[i]);
        vfs_async_finalize(req, req->size);
    }
}

void __attribute__((noreturn)) tty_driver_thread_read() {
    for (;;) {
        semaphore_down(&list_read_sema);
        struct vfs_async_req_t* req = req_read_list.first->val;

        req->size = MIN(req->size, TTY_BUFF_SIZE);
        while (!ringbuff_length(&read_rb)) {
            ASSERT(!(req->flags & VFS_ASYNC_FLAG_WANT_WOULDBLOCK));
            semaphore_down(&read_data_signal);
        }
        size_t res = ringbuff_read(&read_rb, req->vbuff, req->size);
        vfs_async_finalize(req, res);
    }
}

ssize_t tty_submit_req(struct vfs_async_req_t* req) {
    semaphore_down(&insert_lock);
    if (req->type == VFS_ASYNC_TYPE_READ) {
        if (req->flags & VFS_ASYNC_FLAG_WANT_WOULDBLOCK && read_queue_buff >= ringbuff_length(&read_rb)) {
            semaphore_up(&insert_lock);
            return -EWOULDBLOCK;
        }
        list_append(&req_read_list, req);
        semaphore_up(&list_write_sema);
    } else if (req->type == VFS_ASYNC_TYPE_WRITE) {
        list_append(&req_write_list, req);
        semaphore_up(&list_write_sema);
    } else {
        semaphore_up(&insert_lock);
        return -ENOSUP;
    }
    semaphore_up(&insert_lock);
    return 0;
}

void tty_direct_write(char* buff, size_t len) {
    struct semaphore* lock = kmalloc(sizeof(struct semaphore));
    struct vfs_async_req_t* req = kmalloc(sizeof(struct vfs_async_req_t));

    req->type = VFS_ASYNC_TYPE_WRITE;
    req->pid = 0;
    req->req_id = 0;
    req->callback = NULL;
    req->vbuff = buff;
    req->size = len;
    req->flags = 0;
    req->fin_sema = lock;
    semaphore_init(lock);
    
    tty_submit_req(req);
    semaphore_down(lock); // maybe no need to lock, only kfree on callback?

    kfree(req);
    kfree(lock);
}

void tty_mnt_vfs() {
    const struct vfs_reg reg = {
            tty_fs_get_fid,
            tty_submit_req,
            VFS_REG_FLAG_KERNEL_BUFFER_ONLY
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
    
    semaphore_init(&insert_lock);
    semaphore_up(&insert_lock);
    semaphore_init(&list_read_sema);
    semaphore_init(&list_write_sema);
    semaphore_init(&read_data_signal);
    list_init(&req_read_list);
    list_init(&req_write_list);
}

void tty_register_thread() {
    make_kernel_thread("drv::tty::read", tty_driver_thread_read);
    make_kernel_thread("drv::tty::write", tty_driver_thread_write);
}
