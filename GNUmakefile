APP =		nrf9160-boot
ARCH =		arm

CC =		${CROSS_COMPILE}gcc
LD =		${CROSS_COMPILE}ld
OBJCOPY =	${CROSS_COMPILE}objcopy

LDSCRIPT =	${CURDIR}/ldscript

OBJECTS =							\
		errata.o					\
		main.o						\
		osfive/sys/arm/nordicsemi/nrf_uarte.o		\
		osfive/sys/arm/nordicsemi/nrf9160_power.o	\
		osfive/sys/arm/nordicsemi/nrf9160_spu.o		\
		osfive/sys/arm/nordicsemi/nrf9160_timer.o	\
		osfive/sys/arm/nordicsemi/nrf9160_uicr.o	\
		osfive/sys/kern/subr_startup.o			\
		start.o

LIBRARIES = KERN ARM LIBC

CFLAGS =-mthumb -mcpu=cortex-m4 -g -nostdlib -nostdinc	\
	-fno-builtin-printf -ffreestanding		\
	-Wredundant-decls -Wnested-externs		\
	-Wstrict-prototypes -Wmissing-prototypes	\
	-Wpointer-arith -Winline -Wcast-qual		\
	-Wundef -Wmissing-include-dirs -Wall -Werror

all:	__compile __link

clean:	__clean

include osfive/lib/kern/Makefile.inc
include osfive/lib/libc/Makefile.inc
include osfive/mk/gnu.mk
