#include "log.h"

#include <string.h>
#include <driver/tty.h>
#include <driver/serial.h>
#include <libk/kprintf.h>
#include <proc/sched.h>
#include <irq/timer.h>

static int LOG_TARGET_MASK = 0;

#define LOG_BUFF_SIZE 512

void internal_log(char* msg, char* opt_fn_name, int opt_fn_line, int opt_err_level, ...) {
    char log_buff[LOG_BUFF_SIZE];

    char* thread_name = current_proc->name;
    if(!scheduling_enabled)
            thread_name = "kernel";

    if(opt_fn_name == NULL)
        opt_fn_name = "";

    char* err_string = "";
    if(opt_err_level == 1)
        err_string = "{W}";
    else if (opt_err_level == 2)
        err_string = "{E}";
    else if (opt_err_level == 4)
        err_string = "{INTD}";

    char line_buff[5];
    line_buff[0] = '\0';
    if(opt_fn_line!= 0)
        _utoa(line_buff, opt_fn_line, 10);

    va_list args;
    va_start(args, opt_err_level);

    char* log_ptr = log_buff;
    log_ptr += sprintf(log_ptr, "%d: %s[%s(%d)%s%s]: ", (int)sys_ticks, err_string, thread_name, current_proc->pid, opt_fn_name, line_buff);
    log_ptr += vsprintf(log_ptr, msg, args);
    strcpy(log_ptr, "\n");

    va_end(args);

    if (LOG_TARGET_MASK & LOG_TARGET_TTY) {
        if (opt_err_level == 4)
            tty_puts(log_buff); // blocking is not allowed in interrupt disabled sections
        else
            tty_direct_write(log_buff, log_ptr - log_buff + 1);
    }
    if (LOG_TARGET_MASK & LOG_TARGET_SERIAL) {
        char* log_buff_p = log_buff;
        while (*log_buff_p)
            serial_direct_putc(*log_buff_p++); // TODO: blocking fs device write
    }
}

/*
// log
kprintf("1.22 [kauditd(3)]: Guards OK, Kernel Heap 10 kB free");
// FNLOG
kprintf("1.22 [driver:sd(2) kstart:43]: Starting sd driver thread");
// warn
kprintf("1.22 {W}[kauditd(3)]: 1kB left");
// error
kprintf("1.22 {E}[kauditd(3)]: Guard FAILED");
*/

void log_set_target(int target, int enable) {
    if (enable)
        LOG_TARGET_MASK |= target;
    else
        LOG_TARGET_MASK &= target;
}

void log_early_putc(char c) {
    if (LOG_TARGET_MASK & LOG_TARGET_SERIAL)
        serial_direct_putc(c);

    if (LOG_TARGET_MASK & LOG_TARGET_TTY)
        tty_putc(c);
}

void log_early_puts(char* str) {
    while (*str)
        log_early_putc(*str++);
}

void log_early_putx(unsigned int x) {
    for (int i=3; i>=0; i--) {
        int seg = (x>>(i*4)) & 0xf;
        if (seg < 10)
            log_early_putc('0' + seg);
        else
            log_early_putc('A' + seg-10);
    }
} 
