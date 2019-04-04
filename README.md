# nrf9160-boot

A secure bootloader for nRF9160.

Connect UART pins as follows:

| nRF9160          | UART-to-USB adapter  |
| ----------------- | -------------------- |
| P0.29 (TX)        | RX                   |

UART baud rate: 115200

Connect SWD pins as follows:

| nRF9160           | Segger JLINK adapter |
| ----------------- | -------------------- |
| SWDIO             | SWDIO                |
| SWDCLK            | SWCLK                |
| Ground            | Ground               |

### Under Linux
    $ sudo apt install gcc-arm-linux-gnueabi
    $ export CROSS_COMPILE=arm-linux-gnueabi-
### Under FreeBSD
    $ sudo pkg install arm-none-eabi-gcc arm-none-eabi-binutils
    $ export CROSS_COMPILE=arm-none-eabi-
### Build
    $ git clone --recursive https://github.com/osfive/nrf9160-boot
    $ cd nrf9160-boot
    $ make

### Build openocd
    $ sudo apt install pkg-config autotools-dev automake libtool
    $ git clone https://github.com/bukinr/openocd-nrf9160
    $ cd openocd-nrf9160
    $ ./bootstrap
    $ ./configure --enable-jlink
    $ make

### Program the chip
    $ export OPENOCD_PATH=/path/to/openocd-nrf9160
    $ sudo ${OPENOCD_PATH}/src/openocd -s ${OPENOCD_PATH}/tcl \
      -f interface/jlink.cfg -c 'transport select swd; adapter_khz 1000' \
      -f target/nrf9160.cfg -c "program nrf9160-boot.elf 0 reset verify exit"
