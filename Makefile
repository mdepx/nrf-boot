APP =		nrf9160-boot
MACHINE =	arm

CC =		${CROSS_COMPILE}gcc
LD =		${CROSS_COMPILE}ld
OBJCOPY =	${CROSS_COMPILE}objcopy

LDSCRIPT =	${CURDIR}/ldscript

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

all:	_compile _link

clean:	_clean

include osfive/mk/default.mk
include osfive/mk/gnu-toolchain.mk
