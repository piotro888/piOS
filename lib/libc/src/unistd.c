#include "unistd.h"
#include <sys/sys.h>
#include "stdio.h"

void sleep(unsigned long ticks) {
    unsigned long time;
    sys_clockticks(&time);
    
    void (*saved_handler)(struct signal*, int async) =
        sys_sigaction(NULL); // disable handler

    unsigned long end = time+ticks;
    sys_alarmset(end);

    struct signal sigwait;
    do {
        sys_sigwait(&sigwait);
        // end on any non-alarm signal, and skip other alarms
    } while (!(sigwait.type != 2 || (sigwait.type == 2 && sigwait.number == (unsigned int) end)));
    
    sys_alarmset(0); // cancel

    if (sigwait.type != 2 && saved_handler)
        saved_handler(&sigwait, 0); // forward any other signals to handlers in sync mode

    sys_sigaction(saved_handler); // restore signal handling
}
