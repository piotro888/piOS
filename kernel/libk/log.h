#ifndef LIBK_LOG_H
#define LIBK_LOG_H

void internal_log(char* msg, char* opt_fn_name, int opt_fn_line, int opt_err_level, ...);

#define log(x, ...) internal_log((x), 0, 0, 0, ##__VA_ARGS__)
#define log_warn(x, ...) internal_log(x, 0, 0, 1, ##__VA_ARGS__)
#define log_err(x, ...) internal_log(x, 0, 0, 2, ##__VA_ARGS__)
#define log_fn(x, ...) internal_log((x), " "__FILE__":", __LINE__, 0, ##__VA_ARGS__)
#define log_if(enable, x, ...) (enable) && log((x), 0, 0, 0, ##__VA_ARGS__)
#define log_irq(x, ...) internal_log((x), 0, 0, 4, ##__VA_ARGS__)

void log_early_puts(char* str);
void log_early_putc(char c);
void log_set_target(int target, int enable);

#define LOG_TARGET_TTY (1<<0)
#define LOG_TARGET_SERIAL (1<<1)

#endif
