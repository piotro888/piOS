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
};

// fcntl
#define O_NONBLOCK 1

// sig

struct signal {
    unsigned type;
    unsigned number;
};


#endif
