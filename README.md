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

### Set up your environment
    $ sudo apt install gcc-arm-linux-gnueabi
    $ export CROSS_COMPILE=arm-linux-gnueabi-

### Get sources and build
    $ git clone --recursive https://github.com/machdep/nrf-boot
    $ cd nrf-boot
    $ make clean all

## Program the nrf91 chip using nrfjprog
    $ make flash-nrf91

## Program the nrf53 chip using nrfjprog
    $ make flash-nrf53
