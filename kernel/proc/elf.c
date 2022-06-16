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
void load_to_page(int fd, size_t off, size_t end_addr, int page, int prog) {
    ASSERT(end_addr < PAGE_SIZE);

    size_t m_addr = off;
    while (m_addr <= end_addr) {
        size_t read = vfs_read(fd, load_buff, MIN(LOAD_BUFF, (end_addr - m_addr + (prog ? 2 : 1))*(prog ? 2 : 1)));
        if(prog)
            read /= 2; // get size in memory address
        ASSERT(read);

        if(prog)
            load_into_userspace_program(page, load_buff, read, m_addr);
        else
            load_into_userspace(page, load_buff, read, m_addr);

        m_addr += read;
    }
}

void zero_page(size_t off, size_t len, int page) {
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
    int virt_page = (vaddr >> 12);

    if(prog) {
        // this is program virtual LOAD address. LSB of virt_page is used as MSB of lower addr part after disabling
        // programming mode. We are writing exec page mapping, where whole address is shifted >> 1 (including page).
        // 0xxx z*12 -> xxxz z*11 (h/l sel); HIGH/LOW word select is handled by caller which supplies load addr.
        if (proc->prog_pages[virt_page/2] == 0xff)
            proc->prog_pages[virt_page/2] = first_free_prog_page++;

        // We shall return LOAD phys page, which is assigned phys page << 1 | MSB of normal address (which is shifted in LOAD to page section)
        return (proc->prog_pages[virt_page / 2] << 1) | (virt_page & 1);
    } else {
        // load to ram mem is the same as read
        if (proc->mem_pages[virt_page] == 0xff)
            proc->mem_pages[virt_page] = first_free_page++;

        return proc->mem_pages[virt_page];
    }
}

void load_ph(int fd, char* ph, struct proc* proc) {
    for(size_t i=0; i<GET_U16(buff, EH_OFF_PHNUM); i++) {
        vfs_read(fd, ph, PH_SIZE);
        size_t next_ph = vfs_seek(fd, 0, SEEK_CUR);

        if(GET_U16(ph, PH_OFF_TYPE) == PT_LOAD) {
            u16 flags = GET_U16(ph, PH_OFF_FLAGS);
            int prog = (flags & PF_X) > 0;
            int mem_div = (prog ? 4 : 1);

            vfs_seek(fd, GET_U16(ph, PH_OFF_OFFSET), SEEK_SET);
            size_t mem_size = GET_U16(ph, PH_OFF_FILESZ)/mem_div;
            size_t zero_mem_size = GET_U16(ph, PH_OFF_MEMSZ)/mem_div;

            size_t vaddr = GET_U16(ph, PH_OFF_VADDR);
            size_t mem_start = vaddr;
            size_t end_addr = vaddr+mem_size-1;
            int pages = (mem_size+PAGE_SIZE-1)/PAGE_SIZE;

            if(flags&PF_X) {
                // if we are using program page load, we need to supply LOAD address. See comment in phys_page_to_load()
                vaddr *= 2;
                end_addr *= 2;
            }

            if(mem_size) {
                // Load single pages
                load_to_page(fd, vaddr % PAGE_SIZE, MIN(PAGE_SIZE - 1, end_addr), phys_page_to_load(proc, vaddr, prog),
                             prog);
                vaddr += MIN(PAGE_SIZE - (GET_U16(ph, PH_OFF_VADDR) % PAGE_SIZE), end_addr + 1);
                for (int j = 0; j < pages - 2; j++) {
                    load_to_page(fd, 0x000, PAGE_SIZE - 1, phys_page_to_load(proc, vaddr, prog), prog);
                    vaddr += PAGE_SIZE;
                }
                if (pages > 1) {
                    load_to_page(fd, 0x000, end_addr % PAGE_SIZE, phys_page_to_load(proc, vaddr, prog), prog);
                    vaddr += (end_addr % PAGE_SIZE) + 1;
                }
            }

            if(!prog && mem_size < zero_mem_size) {
                while (vaddr < mem_start+zero_mem_size) {
                    size_t size = MIN((PAGE_SIZE-vaddr), mem_start+zero_mem_size-vaddr);
                    zero_page(vaddr%PAGE_SIZE, size, phys_page_to_load(proc, vaddr, prog));
                    vaddr += size;
                }
            }

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
        log("ELF: Invalid target machine (expected pcpu:0x888)");
        return -1;
    }

    u16 phoff = GET_U16(buff, EH_OFF_PHOFF);

    // Load program
    struct proc* proc = sched_init_user_thread();

    proc->pc = GET_U16(buff, EH_OFF_ENTRY);

    vfs_seek(fd, phoff, SEEK_SET);
    load_ph(fd, ph_buff, proc);

    proc->state = PROC_STATE_RUNNABLE;

    return 0;
}
