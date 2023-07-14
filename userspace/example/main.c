#include <sys.h>
#include <shared/syslist.h>

int main() {
    int x = 1;
    int sys_ret;
    char* test = "Hello from userspace!\n";
    int s_size = 23;

    char* dev_tty_name = "/dev/log";
    int dev_size = 8;
    int fd = syscall_raw(SYS_OPEN, (int)dev_tty_name, dev_size, 0, 0);

    sys_ret = syscall_raw(SYS_WRITE, fd, (int)test, s_size, 0);

    // char read_buff[20];
    // sys_ret = syscall_raw(4, fd, read_buff, 20, 0);
    // sys_ret = syscall_raw(5, fd, read_buff, sys_ret, 0);

    // sys_ret = syscall_raw(4, fd, read_buff, 20, 0);
    // sys_ret = syscall_raw(5, fd, read_buff, sys_ret, 0);
    
    for(;;);
}
