#ifndef __SYS_SYSSTRUCTS_H
#define __SYS_SYSSTRUCTS_H

// procinfo
struct sys_proc_info {
    unsigned pid;
    unsigned short type;
    unsigned short state;

    void* load_brk;
    unsigned mem_pages_mapped;
    unsigned prog_pages_mapped;

    unsigned rc;
};

// fcntl
#define F_FLAGS 0
#define F_DRIVER_CUSTOM_START 0x100
#define O_NONBLOCK 1

// sig

struct signal {
    unsigned type;
    unsigned number;
};

// mq
struct msg {
    unsigned id;
    unsigned type;
    unsigned size;
    char data[];
};

#endif
