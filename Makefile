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

flash-nrf53:
	nrfjprog -f NRF53 --erasepage 0x0-0x20000
	nrfjprog -f NRF53 --program obj/nrf53-boot.hex -r

flash-nrf91:
	nrfjprog -f NRF91 --erasepage 0x0-0x20000
	nrfjprog -f NRF91 --program obj/nrf91-boot.hex -r

include ${OSDIR}/mk/user.mk
