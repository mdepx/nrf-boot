/*-
 * Copyright (c) 2018-2019 Ruslan Bukin <br@bsdpad.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#include <sys/console.h>
#include <sys/systm.h>
#include <sys/malloc.h>

#include <arm/arm/nvic.h>
#include <arm/arm/scb.h>
#include <arm/nordicsemi/nrf9160.h>

#include <machine/cpuregs.h>

struct uarte_softc uarte_sc;
struct arm_nvic_softc nvic_sc;
struct spu_softc spu_sc;
struct power_softc power_sc;
struct arm_scb_softc scb_sc;

#define	UART_PIN_TX	29
#define	UART_PIN_RX	21
#define	UART_BAUDRATE	115200

#define	APP_ENTRY	0x40000

void app_main(void);
void jump_ns(uint32_t addr);

extern uint32_t _smem;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

static void
uart_putchar(int c, void *arg)
{
	struct uarte_softc *sc;
 
	sc = arg;
 
	if (c == '\n')
		uarte_putc(sc, '\r');

	uarte_putc(sc, c);
}

static void
clear_bss(void)
{
	uint8_t *sbss;
	uint8_t *ebss;

	sbss = (uint8_t *)&_sbss;
	ebss = (uint8_t *)&_ebss;

	while (sbss < ebss)
		*sbss++ = 0;
}

static void
copy_sdata(void)
{
	uint8_t *dst;
	uint8_t *src;

	/* Copy sdata to RAM if required */
	for (src = (uint8_t *)&_smem, dst = (uint8_t *)&_sdata;
	    dst < (uint8_t *)&_edata; )
		*dst++ = *src++;
}

static void
secure_boot_configure(void)
{
	int i;

	for (i = 0; i < 8; i++)
		spu_flash_set_perm(&spu_sc, i, 1);
	for (i = 8; i < 32; i++)
		spu_flash_set_perm(&spu_sc, i, 0);

	for (i = 0; i < 8; i++)
		spu_sram_set_perm(&spu_sc, i, 1);
	for (i = 8; i < 32; i++)
		spu_sram_set_perm(&spu_sc, i, 0);

	spu_gpio_set_perm(&spu_sc, 0, 0);

	spu_periph_set_attr(&spu_sc, ID_CLOCK, 0, 0);
	spu_periph_set_attr(&spu_sc, ID_RTC1, 0, 0);
	spu_periph_set_attr(&spu_sc, ID_IPC, 0, 0);
	spu_periph_set_attr(&spu_sc, ID_NVMC, 0, 0);
	spu_periph_set_attr(&spu_sc, ID_VMC, 0, 0);
	spu_periph_set_attr(&spu_sc, ID_GPIO, 0, 0);
	spu_periph_set_attr(&spu_sc, ID_GPIOTE1, 0, 0);
	spu_periph_set_attr(&spu_sc, ID_UARTE1, 0, 0);
	spu_periph_set_attr(&spu_sc, ID_EGU1, 0, 0);
	spu_periph_set_attr(&spu_sc, ID_EGU2, 0, 0);
	spu_periph_set_attr(&spu_sc, ID_FPU, 0, 0);
	spu_periph_set_attr(&spu_sc, ID_TWIM2, 0, 0);
	spu_periph_set_attr(&spu_sc, ID_SPIM3, 0, 0);
}

void
app_main(void)
{
	uint32_t control_ns;
	uint32_t msp_ns;
	uint32_t psp_ns;
	uint32_t *vec;

	clear_bss();
	copy_sdata();

	uarte_init(&uarte_sc, BASE_UARTE0 | PERIPH_SECURE_ACCESS,
	    UART_PIN_TX, UART_PIN_RX, UART_BAUDRATE);
	console_register(uart_putchar, (void *)&uarte_sc);

	printf("Hello world!\n");

	spu_init(&spu_sc, BASE_SPU);

	secure_boot_configure();

	arm_scb_init(&scb_sc, BASE_SCS);
	arm_scb_set_vector(&scb_sc, APP_ENTRY);

	vec = (uint32_t *)APP_ENTRY;

	msp_ns = vec[0];
	psp_ns = 0;

	__asm __volatile("msr msp_ns, %0" :: "r" (msp_ns));
	__asm __volatile("msr msp_ns, %0" :: "r" (psp_ns));

	__asm __volatile("mrs %0, control_ns" : "=r" (control_ns));
	control_ns &= ~CONTROL_SPSEL; /* Use main stack pointer. */
	control_ns &= ~CONTROL_NPRIV; /* Set privilege mode. */
	__asm __volatile("msr control_ns, %0" :: "r" (control_ns));

	arm_scb_exceptions_prio_config(&scb_sc, 1);
	arm_scb_exceptions_target_config(&scb_sc, 0);
	arm_scb_sysreset_secure(&scb_sc, 0);
	arm_sau_configure(&scb_sc, 0, 1);
	arm_fpu_non_secure(&scb_sc, 1);

	printf("Jumping to non-secure address 0x%x\n", vec[1]);

	spu_periph_set_attr(&spu_sc, ID_UARTE0, 0, 0);

	jump_ns(vec[1]);

	/* UNREACHABLE */
}
