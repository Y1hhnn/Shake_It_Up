#ifndef PROTO_H
#define PROTO_H

#include <stdint.h>

void proto_init(void);   
void proto_putc(char ch);
void proto_puts(char *ptr_str);
char proto_getc_nonblocking(void);

#endif
