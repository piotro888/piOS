/* 
 * piOS kernel entry point
 * (C) 2021 by Piotr Wegrzyn 
*/

#include <video/tty.h>

/* C entry point for kernel */
void _kstart(){
   
    init_tty();
    while(1) {
        tty_putc('a');
        tty_putc('b');
        tty_putc('c');
        tty_putc('d');
        tty_putc('e');
        tty_print_buffer();
    }
}
