#include "kbd.h"
#include <fs/vfs.h>
#include <libk/types.h>

/* Temporary vfs device hooked up
 * to PS/2 keyboard driver.
 * Only for testing purposes of vfs 
 */

char c_buff[16];
uint8_t c_buff_pos = 0;
uint8_t c_buff_read_pos = 0;

const char* dev_name = "/dev/kbd";

int8_t kbd_open(char* path) {
    return 0;
}

int8_t kbd_close(char* path) {
    return 0;
}

size_t kbd_read(char* buff, size_t len) {
    size_t size_read = 0;
    while(c_buff_read_pos != c_buff_pos) {
        *buff = c_buff[c_buff_read_pos];
        size_read++;

        if(c_buff_read_pos != 0)
            c_buff_read_pos--;
        else
            c_buff_read_pos = 15;
    }
    return size_read;
}

size_t kbd_write() {
    return 0;
}

void kbd_vfs_submit_char(char c) {
    c_buff[c_buff_pos] = c;
    c_buff_pos++;
    if(c_buff_pos == 16) {
        c_buff_pos = 0;
    }
}


struct vfs_reg_t kbd_vfs_reg = {
    kbd_open,
    kbd_close,
    kbd_read,
    kbd_write
};


void kbd_vfs_init() {
    vfs_mount(dev_name, &kbd_vfs_reg);
}