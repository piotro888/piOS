#include "elf.h"

#include <string.h>
#include <fs/vfs.h>
#include <proc/virtual.h>
#include <proc/sched.h>
#include <libk/assert.h>
#include <libk/log.h>
#include <libk/math.h>
#include <libk/types.h>

#define ELFCLASS32  1
#define ELFDATA2LSB 1
#define EV_CURRENT  1
#define ET_EXEC     2
#define EM_PCPU     0x0888

#define HEADER_SIZE 56
#define EH_OFF_TYPE 16
#define EH_OFF_MACHINE 18
#define EH_OFF_ENTRY 24
#define EH_OFF_PHOFF 28
#define EH_OFF_PHNUM 44

struct elf_header {
    u8 e_ident[16];
    u16 e_type; //16
    u16 e_machine; //18
    u32 e_version; //20
    u32 e_entry; //24
    u32 e_phoff; //28
    u32 e_shoff; //32
    u32 e_flags; //36
    u16 e_ehsize;//40
    u16 e_phentsize; //42
    u16 e_phnum; //44
    u16 e_shentsize; //46
    u16 e_shnum; //48
    u16 e_shstrndx; //50
};

#define PT_LOAD     1
#define PF_X        1
#define PF_W        2
#define PF_R        4

#define PH_SIZE 32
#define PH_OFF_TYPE 0
#define PH_OFF_OFFSET 4
#define PH_OFF_VADDR 8
#define PH_OFF_FILESZ 16
#define PH_OFF_MEMSZ 20
#define PH_OFF_FLAGS 24
#define PH_OFF_ALIGN 28

struct elf_program_header {
    u32 p_type;
    u32 p_offset;
    u32 p_vaddr;
    u32 p_paddr;
    u32 p_filesz;
    u32 p_memsz;
    u32 p_flags;
    u32 p_align;
};

#define LOAD_BUFF 0x100
static char load_buff[LOAD_BUFF];

static char buff[HEADER_SIZE];
static char ph_buff[PH_SIZE];

// Casting buff to struct doesn't work due to gcc port, we need to use this :(
#define GET_U8(buff, off) (buff)[(off)]
#define GET_U16(buff, off) ((((u16)((buff)[(off)+1]))<<8u) | (u16)((buff)[(off)]))
#define GET_U32(buff, off) ((((u32)((buff)[(off)+3]))<<24ul) | (((u32)((buff)[(off)+2]))<<16ul) | \
                            (((u32)((buff)[(off)]))<<8ul) | (u32)((buff)[(off)]))

// address is memory address
void load_to_page(struct proc_file* file, size_t off, size_t end_addr, int page, int prog) {
    ASSERT(end_addr < PAGE_SIZE);
    log_dbg("load off=%x end=%x page=%x prog=%x", off, end_addr, page, prog);

    size_t m_addr = off;
    while (m_addr <= end_addr) {
        size_t read = vfs_read_blocking(file, 0, load_buff, MIN(LOAD_BUFF, (end_addr - m_addr + 1)));
        ASSERT(read);

        if(prog)
            load_into_userspace_program(page, load_buff, read, m_addr);
        else
            load_into_userspace(page, load_buff, read, m_addr);

        m_addr += read;
    }
}

void zero_page(size_t off, size_t len, int page) {
    log_dbg("load bss off=%x end=%x page=%x prog=%x", off, len, page);
    ASSERT(off+len <= PAGE_SIZE);

    map_page_zero(page);
    for(size_t i=0; i<len; i++) {
        // HACK: Access every address (*u16++ would not work)
        *((u16*)off) = 0;
        off++;
    }
    map_page_zero(0);
}

int phys_page_to_load(struct proc* proc, u16 vaddr, int prog) {
    unsigned int virt_page = (vaddr >> 12);

    if(prog) {
        // PC addressing works differently. We need to return page from DATA memory that maps INSTR memory.
        
        // INSTR PAGE entry is 11 bit wide MEMORY PAGE ENTRY 12 bit wide
        // So, instruction page index is vaddr>>11
        if (proc->proc_state.prog_pages[virt_page/2] == 0xff)
            proc->proc_state.prog_pages[virt_page/2] = first_free_prog_page++;
        log_dbg("prog page virt_page=%x dst=%x ffp=%x pp[%x]=%x", virt_page, proc->proc_state.prog_pages[virt_page], first_free_page, virt_page/2, proc->proc_state.prog_pages[virt_page/2]);
        
        // We shall return corresponding DATA page, which is: assigned phys page << 1 | MSB of non-page part in INSTR space + INSTR region start page
        return ((proc->proc_state.prog_pages[virt_page / 2] << 1) | (virt_page & 1)) + (0x800<<1);
    } else {
        // load to ram mem is the same as read
        if (proc->proc_state.mem_pages[virt_page] == 0xff)
            proc->proc_state.mem_pages[virt_page] = first_free_page++;
        log_dbg("mem page virt_page=%x dst=%x ffp=%x", virt_page, proc->proc_state.mem_pages[virt_page], first_free_page);
        return proc->proc_state.mem_pages[virt_page];
    }
}

void load_ph(struct proc_file* file, char* ph, struct proc* proc) {
    size_t load_brk = 0;
    for(size_t i=0; i<GET_U16(buff, EH_OFF_PHNUM); i++) {
        vfs_read_blocking(file, 0, ph, PH_SIZE);
        size_t next_ph = vfs_seek(file, 0, SEEK_CUR);

        if(GET_U16(ph, PH_OFF_TYPE) == PT_LOAD) {
            u16 flags = GET_U16(ph, PH_OFF_FLAGS);
            int prog = (flags & PF_X) != 0;

#define DEBUG
            vfs_seek(file, GET_U16(ph, PH_OFF_OFFSET), SEEK_SET);
            size_t mem_size = GET_U16(ph, PH_OFF_FILESZ);
            size_t zero_mem_size = GET_U16(ph, PH_OFF_MEMSZ);

            size_t vaddr = GET_U16(ph, PH_OFF_VADDR);
            size_t mem_start = vaddr;
            size_t end_addr = vaddr+mem_size-1;
            int pages = (mem_size+PAGE_SIZE-1)/PAGE_SIZE;

            log_dbg("hdr: memsz %x vaddr %x %x mem_start %x end_addr %x", mem_size, vaddr, vaddr % PAGE_SIZE, mem_start, end_addr);
            
            if(!prog && zero_mem_size)
                load_brk = MAX(load_brk, vaddr+zero_mem_size);

            // all adresses are in byte indexed memory (and are written in that manner). 1 instruction takes 4 bytes
            // PAGE_SIZE is a limit of page map to load in all cases (byte addressed)

            if(mem_size) {
                // Load single pages - first page, then full pages, last page
                load_to_page(file, vaddr % PAGE_SIZE, MIN(PAGE_SIZE - 1, end_addr-vaddr), phys_page_to_load(proc, vaddr, prog), prog);
                vaddr += MIN(PAGE_SIZE - (GET_U16(ph, PH_OFF_VADDR) % PAGE_SIZE), end_addr + 1);

                for (int j = 0; j < pages - 2; j++) {
                    load_to_page(file, 0x000, PAGE_SIZE - 1, phys_page_to_load(proc, vaddr, prog), prog);
                    vaddr += PAGE_SIZE;
                }
                
                if (pages > 1) {
                    load_to_page(file, 0x000, end_addr % PAGE_SIZE, phys_page_to_load(proc, vaddr, prog), prog);
                    vaddr += (end_addr % PAGE_SIZE) + 1;
                }
            }

            if(!prog && mem_size < zero_mem_size) {
                while (vaddr < mem_start+zero_mem_size) {
                    size_t size = MIN((PAGE_SIZE - (vaddr % PAGE_SIZE)), mem_start+zero_mem_size-vaddr);
                    zero_page(vaddr%PAGE_SIZE, size, phys_page_to_load(proc, vaddr, prog));
                    vaddr += size;
                }
            }
        }
        vfs_seek(file, next_ph, SEEK_SET);
    }
    proc->load_brk = (void*)load_brk;
    log_dbg("elf break: 0x%x", proc->load_brk);
}

struct proc* elf_load(struct proc_file* file) {
    if (vfs_read_blocking(file, 0, buff, HEADER_SIZE) != HEADER_SIZE) {
        log("Elf: file too short");
        return NULL;
    }

    if (GET_U8(buff, 0) != 0x7f || GET_U8(buff, 1)  != 'E' ||
        GET_U8(buff, 2)  != 'L' || GET_U8(buff, 3)  != 'F') {
        log("ELF: Bad header");
        return NULL;
    }

    if (GET_U8(buff, 4) != ELFCLASS32 || GET_U8(buff, 5) != ELFDATA2LSB || GET_U8(buff, 6) != EV_CURRENT) {
        log("ELF: Unsupported format");
        return NULL;
    }

    if (GET_U16(buff, EH_OFF_TYPE) != ET_EXEC) {
        log("ELF: Not an executable");
        return NULL;
    }

    if (GET_U16(buff, EH_OFF_MACHINE) != EM_PCPU) {
        log("ELF: Invalid target machine (expected pcpu:0x888)");
        return NULL;
    }

    u16 phoff = GET_U16(buff, EH_OFF_PHOFF);

    // Load program
    struct proc* proc = sched_init_user_thread();

    strcpy(proc->name, file->inode->name);

    proc->proc_state.pc = GET_U16(buff, EH_OFF_ENTRY);

    vfs_seek(file, phoff, SEEK_SET);
    load_ph(file, ph_buff, proc);

    return proc;
}
