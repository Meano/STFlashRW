#ifndef __USARTH
#define __USARTH

#include "Root.h"
#include <stdio.h>

#ifdef __GNUC__
	#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
	#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

extern void USARTInitialize(void);

#endif
