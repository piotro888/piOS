#include "log.h"

#include <driver/tty.h>
#include <libk/kprintf.h>
#include <libk/string.h>
#include <proc/sched.h>

#define LOG_TARGET_VGA 0
int LOG_TARGET = LOG_TARGET_VGA;

unsigned int log_time = 0;

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

    char line_buff[5];
    line_buff[0] = '\0';
    if(opt_fn_line!= 0)
        utoa(line_buff, opt_fn_line, 10);

    va_list args;
    va_start(args, opt_err_level);

    char* log_ptr = log_buff;
    log_ptr += sprintf(log_ptr, "%d.%d: %s[%s(%d)%s%s]: ", log_time/100, log_time%100, err_string, thread_name, current_proc->pid, opt_fn_name, line_buff);
    log_ptr += vsprintf(log_ptr, msg, args);
    strcpy(log_ptr, "\n");

    va_end(args);

    if(LOG_TARGET == LOG_TARGET_VGA)
        tty_puts(log_buff);
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
