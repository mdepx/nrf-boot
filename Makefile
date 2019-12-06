APP =		nrf-boot

OSDIR =		mdepx

export CFLAGS = -mthumb -mcpu=cortex-m4 -g -nostdlib	\
	-nostdinc -fno-builtin-printf -ffreestanding	\
	-Wredundant-decls -Wnested-externs		\
	-Wstrict-prototypes -Wmissing-prototypes	\
	-Wpointer-arith -Winline -Wcast-qual		\
	-Wundef -Wmissing-include-dirs -Wall -Werror

all:
	@echo Run make nrf53 or make nrf91

nrf53:
	@python3 -B ${OSDIR}/tools/emitter.py mdepx-nrf53.conf
	@${CROSS_COMPILE}objcopy -O ihex obj/${APP}.elf obj/${APP}.hex

nrf91:
	@python3 -B ${OSDIR}/tools/emitter.py mdepx-nrf91.conf
	@${CROSS_COMPILE}objcopy -O ihex obj/${APP}.elf obj/${APP}.hex

clean:
	@rm -rf obj/*

include ${OSDIR}/mk/user.mk
