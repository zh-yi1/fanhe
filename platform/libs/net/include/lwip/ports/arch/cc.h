/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef LWIP_ARCH_CC_H
#define LWIP_ARCH_CC_H

#define LWIP_TIMEVAL_PRIVATE 0
// #define LWIP_ERRNO_STDINCLUDE

// #define LWIP_PROVIDE_ERRNO
#include <errno.h>
int *_os_errno(void);
#undef errno
#define errno (*_os_errno())

#define LWIP_NO_UNISTD_H 1

#include <sys/time.h>
#ifndef  _INCLUDE_H
#include <sys/fcntl.h>

#include <stdlib.h>
void my_printf(const char *format, ...);

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define AT(x)                   __attribute__((section(#x)))
#define ALWAYS_INLINE           __attribute__((always_inline)) inline
#endif

#ifndef INT_MAX
#define INT_MAX    0x7FFFFFFF
#endif

extern uint32_t lwip_port_rand(void);
#define LWIP_RAND() (lwip_port_rand())

struct sio_status_s;
typedef struct sio_status_s sio_status_t;
#define sio_fd_t sio_status_t*
#define __sio_fd_t_defined

// typedef unsigned int sys_prot_t;

#define SSIZE_MAX INT_MAX

#define LWIP_PLATFORM_DIAG(x) do {my_printf x;} while(0)

#define LWIP_PLATFORM_ASSERT(x) do {my_printf("Assertion \"%s\" failed at line %d in %s\n", \
                                     x, __LINE__, __FILE__); lwip_wdt_rst();} while(0)

void lwip_wdt_rst(void);
#define X8_F "x"
#define U16_F "u"
#define S16_F "d"
#define X16_F "x"
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"
#define SZT_F "u"

#endif /* LWIP_ARCH_CC_H */
