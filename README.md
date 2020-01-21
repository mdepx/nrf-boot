# nrf-boot

A secure bootloader for nRF9160 and nRF5340.

Plug a micro USB cable to nRF9160-DK or nRF5340-DK.

For other boards, connect UART pins as follows:

| nRF9160           | UART-to-USB adapter  |
| ----------------- | -------------------- |
| P0.29 (TX)        | RX                   |

| nRF5340           | UART-to-USB adapter  |
| ----------------- | -------------------- |
| P0.20 (TX)        | RX                   |

UART baud rate: 115200

Connect SWD pins as follows:

| nRF9160 or nRF5340 | Segger JLINK adapter |
| ------------------ | -------------------- |
| SWDIO              | SWDIO                |
| SWDCLK             | SWCLK                |
| Ground             | Ground               |

### Under Linux
    $ sudo apt install gcc-arm-linux-gnueabi
    $ export CROSS_COMPILE=arm-linux-gnueabi-

### Under FreeBSD
    $ sudo pkg install arm-none-eabi-gcc arm-none-eabi-binutils
    $ export CROSS_COMPILE=arm-none-eabi-

### Build
    $ git clone --recursive https://github.com/machdep/nrf-boot
    $ cd nrf-boot
    $ make

## Program the nrf91 chip using nrfjprog
    $ nrfjprog -f NRF91 --erasepage 0x0-0x9000
    $ nrfjprog -f NRF91 --program obj/nrf91-boot.hex -r

## Program the nrf53 chip using nrfjprog
    $ nrfjprog -f NRF53 --erasepage 0x0-0x9000
    $ nrfjprog -f NRF53 --program obj/nrf53-boot.hex -r

## Program the chip using OpenOCD

### Build openocd
    $ sudo apt install pkg-config autotools-dev automake libtool
    $ git clone https://github.com/bukinr/openocd-nrf9160
    $ cd openocd-nrf9160
    $ ./bootstrap
    $ ./configure --enable-jlink
    $ make

### Invoke openocd
    $ export OPENOCD_PATH=/path/to/openocd-nrf9160
    $ sudo ${OPENOCD_PATH}/src/openocd -s ${OPENOCD_PATH}/tcl \
      -f interface/jlink.cfg -c 'transport select swd; adapter_khz 1000' \
      -f target/nrf9160.cfg -c "program nrf-boot.elf 0 reset verify exit"
