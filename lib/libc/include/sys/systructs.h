#ifndef __SYS_SYSSTRUCTS_H
#define __SYS_SYSSTRUCTS_H

struct sys_proc_info {
    unsigned pid;
    unsigned short type;
    unsigned short state;
    
    void* load_brk;
    unsigned mem_pages_mapped;
    unsigned prog_pages_mapped;
};


#endif
