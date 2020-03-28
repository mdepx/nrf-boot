APP =		nrf-boot

OSDIR =		mdepx

export CFLAGS = -mthumb -mcpu=cortex-m33 -g -nostdlib	\
	-nostdinc -fno-builtin-printf -ffreestanding	\
	-Wredundant-decls -Wnested-externs		\
	-Wstrict-prototypes -Wmissing-prototypes	\
	-Wpointer-arith -Winline -Wcast-qual		\
	-Wundef -Wmissing-include-dirs -Wall -Werror

export AFLAGS = ${CFLAGS}

all:	nrf53 nrf91

nrf53:
	@python3 -B ${OSDIR}/tools/emitter.py mdepx-nrf53.conf
	@${CROSS_COMPILE}objcopy -O ihex obj/nrf53-boot.elf obj/nrf53-boot.hex

nrf91:
	@python3 -B ${OSDIR}/tools/emitter.py mdepx-nrf91.conf
	@${CROSS_COMPILE}objcopy -O ihex obj/nrf91-boot.elf obj/nrf91-boot.hex

clean:
	@rm -rf obj/*

include ${OSDIR}/mk/user.mk
