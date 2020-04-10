APP =		nrf-boot

OSDIR =		mdepx
EMITTER =	python3 -B ${OSDIR}/tools/emitter.py

all:	nrf53 nrf91

nrf53:
	@${EMITTER} -j mdepx-nrf53.conf
	@${CROSS_COMPILE}objcopy -O ihex obj/nrf53-boot.elf obj/nrf53-boot.hex

nrf91:
	@${EMITTER} -j mdepx-nrf91.conf
	@${CROSS_COMPILE}objcopy -O ihex obj/nrf91-boot.elf obj/nrf91-boot.hex

clean:
	@rm -rf obj/*

include ${OSDIR}/mk/user.mk
