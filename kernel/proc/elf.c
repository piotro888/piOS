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
char load_buff[LOAD_BUFF];
u16 load_buff_c[LOAD_BUFF/2];

char buff[HEADER_SIZE];
char ph_buff[PH_SIZE];

// Casting buff to struct doesn't work due to gcc port, we need to use this :(
#define GET_U8(buff, off) (buff)[(off)]
#define GET_U16(buff, off) ((((u16)((buff)[(off)+1]))<<8u) | (u16)((buff)[(off)]))
#define GET_U32(buff, off) ((((u32)((buff)[(off)+3]))<<24ul) | (((u32)((buff)[(off)+2]))<<16ul) | \
                            (((u32)((buff)[(off)]))<<8ul) | (u32)((buff)[(off)]))
int lpl;
void load_to_page(int fd, size_t off, size_t end_addr, int page) {
    log("ltp %x:%x->%x phys %x virt %x", page, off, end_addr, (page<<11)|(off>>1), (lpl<<11)|(off>>1));
    ASSERT(end_addr < PAGE_SIZE);
    size_t m_addr = off;
    while (m_addr <= end_addr) {
        //size_t read = vfs_read(fd, load_buff, MIN(LOAD_BUFF, (end_addr-m_addr)*2));
        //read /= 2; // get size in memory address
        size_t read = MIN(LOAD_BUFF, (end_addr-m_addr)*2);

        for(size_t i=0; i<read; i++) // compress 8->16
            load_buff_c[i] = GET_U16(load_buff, i*2);

        //load_into_userspace(page, load_buff_c, read, m_addr);

        m_addr += read/2;
    }
}

int page_to_load(struct proc* proc, u16 vaddr) {
    int page = (vaddr>>12);
    // this is load address!! so page should be set for page/2 and load page shall be returned
    // Real page (lsb is ignored, and it is msb of 16b addr)
    if(proc->prog_pages[page/2] == 0xff)
       proc->prog_pages[page/2] = first_free_page++;
    // FIXME: prog pages should be requested from other int
    // Shifted load page
    log("ptl v%x pl%x p%x tp%x tl%x", vaddr, page, page/2, proc->prog_pages[page/2], (proc->prog_pages[page/2]<<1) + (page&1));
    lpl = page;
    return (proc->prog_pages[page/2]<<1) + (page&1);
}

void load_ph(int fd, char* ph, struct proc* proc) {
    first_free_page = 0;
    for(size_t i=0; i<GET_U16(buff, EH_OFF_PHNUM); i++) {
        vfs_read(fd, ph, PH_SIZE);
        size_t next_ph = vfs_seek(fd, 0, SEEK_CUR);

        if(GET_U16(ph, PH_OFF_TYPE) == PT_LOAD) {
            u16 flags = GET_U16(ph, PH_OFF_FLAGS);
            int mem_div = (flags&PF_X ? 2 : 4);

            vfs_seek(fd, GET_U16(ph, PH_OFF_OFFSET), SEEK_SET);
            size_t mem_size = GET_U16(ph, PH_OFF_FILESZ)/mem_div;
            mem_size = 0x4000;

            size_t vaddr = GET_U16(ph, PH_OFF_VADDR);
            vaddr = 0x1af2;
            size_t end_addr = vaddr+mem_size;
            int pages = (mem_size+PAGE_SIZE-1)/PAGE_SIZE;

            if(flags&PF_X) {
                vaddr *= 2;
                end_addr *= 2;
            } else {
                continue;
            }
            log("lph eva %x lva %x pg %x ea %x", vaddr/2, vaddr, pages, end_addr);
            // Load single pages
            load_to_page(fd, vaddr % PAGE_SIZE, MIN(0x0fff, end_addr%PAGE_SIZE), page_to_load(proc, vaddr));
            vaddr += PAGE_SIZE-(GET_U16(ph, PH_OFF_VADDR)%PAGE_SIZE);
            for(int j=0; j<pages-2; j++) {
                load_to_page(fd, 0x000, 0x0fff, page_to_load(proc, vaddr));
                vaddr += 0x1000;
            }
            if(pages > 1)
                load_to_page(fd, 0x000, end_addr % PAGE_SIZE, page_to_load(proc, vaddr));
        }
        vfs_seek(fd, next_ph, SEEK_SET);
    }
}

int elf_load(int fd) {
    if (vfs_read(fd, buff, HEADER_SIZE) != HEADER_SIZE) {
        log("Elf: file too short");
        return -1;
    }

    if (GET_U8(buff, 0) != 0x7f || GET_U8(buff, 1)  != 'E' ||
        GET_U8(buff, 2)  != 'L' || GET_U8(buff, 3)  != 'F') {
        log("ELF: Bad header");
        return -1;
    }

    if (GET_U8(buff, 4) != ELFCLASS32 || GET_U8(buff, 5) != ELFDATA2LSB || GET_U8(buff, 6) != EV_CURRENT) {
        log("ELF: Unsupported format");
        return -1;
    }

    if (GET_U16(buff, EH_OFF_TYPE) != ET_EXEC) {
        log("ELF: Not an executable");
        return -1;
    }

    if (GET_U16(buff, EH_OFF_MACHINE) != EM_PCPU) {
        log("ELF: Invalid target machine");
        return -1;
    }

    u16 phoff = GET_U16(buff, EH_OFF_PHOFF);

    // Load program
    struct proc* proc = sched_init_user_thread();
    proc->pc = GET_U16(buff, EH_OFF_ENTRY);
    vfs_seek(fd, phoff, SEEK_SET);
    load_ph(fd, ph_buff, proc);
    return 0;
}
