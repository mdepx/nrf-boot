APP =		nrf9160-boot

OSDIR =		mdepx

export CFLAGS = -mthumb -mcpu=cortex-m4 -g -nostdlib	\
	-nostdinc -fno-builtin-printf -ffreestanding	\
	-Wredundant-decls -Wnested-externs		\
	-Wstrict-prototypes -Wmissing-prototypes	\
	-Wpointer-arith -Winline -Wcast-qual		\
	-Wundef -Wmissing-include-dirs -Wall -Werror

all:
	@python3 -B ${OSDIR}/tools/emitter.py mdepx.conf
	@${CROSS_COMPILE}objcopy -O ihex obj/${APP}.elf obj/${APP}.hex

clean:
	@rm -rf obj/*

include ${OSDIR}/mk/user.mk
