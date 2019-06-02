APP =		nrf9160-boot
MACHINE =	arm

CC =		${CROSS_COMPILE}gcc
LD =		${CROSS_COMPILE}ld
OBJCOPY =	${CROSS_COMPILE}objcopy

LDSCRIPT =	${CURDIR}/ldscript

OBJDIR =	obj
OSDIR =		mdepx

OBJECTS =							\
		errata.o					\
		main.o						\
		${OSDIR}/sys/arm/nordicsemi/nrf_uarte.o		\
		${OSDIR}/sys/arm/nordicsemi/nrf9160_power.o	\
		${OSDIR}/sys/arm/nordicsemi/nrf9160_spu.o	\
		${OSDIR}/sys/arm/nordicsemi/nrf9160_timer.o	\
		${OSDIR}/sys/arm/nordicsemi/nrf9160_uicr.o	\
		start.o

KERNEL =
LIBRARIES = libc

CFLAGS =-mthumb -mcpu=cortex-m4 -g -nostdlib -nostdinc	\
	-fno-builtin-printf -ffreestanding		\
	-Wredundant-decls -Wnested-externs		\
	-Wstrict-prototypes -Wmissing-prototypes	\
	-Wpointer-arith -Winline -Wcast-qual		\
	-Wundef -Wmissing-include-dirs -Wall -Werror

all:	${OBJDIR}/${APP}.elf

clean:
	@rm -f ${OBJECTS} ${OBJDIR}/{APP}.elf

include ${OSDIR}/lib/libc/Makefile.inc
include ${OSDIR}/mk/default.mk
