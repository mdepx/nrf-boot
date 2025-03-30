/*-
 * Copyright (c) 2018-2020 Ruslan Bukin <br@bsdpad.com>
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
#include <sys/thread.h>

#include <arm/arm/nvic.h>
#include <arm/arm/scb.h>
#include <arm/nordicsemi/nrf9160.h>

#include <machine/cpuregs.h>

#include <dev/gpio/gpio.h>
#include <dev/intc/intc.h>
#include <dev/uart/uart.h>

#include "errata.h"
#include "cc310.h"

static struct arm_nvic_softc nvic_sc;
static struct arm_scb_softc scb_sc;
static struct nrf_uarte_softc uarte_sc;
static struct nrf_spu_softc spu_sc;
static struct nrf_power_softc power_sc;
static struct nrf_gpio_softc gpio0_sc;
static struct nrf_nvmc_softc nvmc_sc;
static struct nrf_uicr_softc uicr_sc;

struct mdx_device dev_nvic  = { .sc = &nvic_sc };
struct mdx_device dev_nvmc  = { .sc = &nvmc_sc };
struct mdx_device dev_scb   = { .sc = &scb_sc };
struct mdx_device dev_spu   = { .sc = &spu_sc };
struct mdx_device dev_uart  = { .sc = &uarte_sc };
struct mdx_device dev_gpio  = { .sc = &gpio0_sc };
struct mdx_device dev_power = { .sc = &power_sc };
struct mdx_device dev_uicr  = { .sc = &uicr_sc };

#define	APP_ENTRY	0x40000

void app_main(void);
void jump_ns(uint32_t addr);

static void
secure_boot_configure_periph(int periph_id)
{

	nrf_spu_periph_set_attr(&dev_spu, periph_id, false, false);
	mdx_intc_disable(&dev_nvic, periph_id);
	arm_nvic_target_ns(&dev_nvic, periph_id, 0);
}

static void
secure_boot_configure(void)
{
	int i;

	/* Configure FLASH */

	nrf_spu_flashnsc(&dev_spu, 0, 0, 32, true);

	for (i = 0; i < 8; i++)
		nrf_spu_flash_set_perm(&dev_spu, i, true);
	for (i = 8; i < 32; i++)
		nrf_spu_flash_set_perm(&dev_spu, i, false);

	/* First 16kb of sram is secure */
	for (i = 0; i < 2; i++)
		nrf_spu_sram_set_perm(&dev_spu, i, true);

	/* The rest of sram is not secure. */
	for (i = 2; i < 32; i++)
		nrf_spu_sram_set_perm(&dev_spu, i, false);

	nrf_spu_gpio_set_perm(&dev_spu, 0, 0);

	secure_boot_configure_periph(ID_CLOCK);
	secure_boot_configure_periph(ID_RTC1);
	secure_boot_configure_periph(ID_IPC);
	secure_boot_configure_periph(ID_NVMC);
	secure_boot_configure_periph(ID_VMC);
	secure_boot_configure_periph(ID_GPIO);
	secure_boot_configure_periph(ID_GPIOTE1);
	secure_boot_configure_periph(ID_UARTE1);
	secure_boot_configure_periph(ID_EGU1);
	secure_boot_configure_periph(ID_EGU2);
	secure_boot_configure_periph(ID_FPU);
	secure_boot_configure_periph(ID_TWIM2);
	secure_boot_configure_periph(ID_SPIM3);
	secure_boot_configure_periph(ID_TIMER0);
}

void
board_init(void)
{

	nrf_uarte_init(&dev_uart, BASE_UARTE0 | PERIPH_SECURE_ACCESS,
	    BOARD_UART_PIN_TX,
	    BOARD_UART_PIN_RX);
	mdx_uart_setup(&dev_uart, BOARD_UART_BAUDRATE, UART_DATABITS_8,
	    UART_STOPBITS_1, UART_PARITY_NONE);
	mdx_console_register_uart(&dev_uart);
}

static void
cc310_intr(void *arg, int irq)
{

	CRYPTOCELL_IRQHandler();
}

void
board_cryptocell_setup(void)
{
	uint32_t reg;

	reg = BASE_CRYPTOCELL + CRYPTOCELL_ENABLE;
	*(volatile uint32_t *)reg = 1;

	mdx_intc_setup(&dev_nvic, ID_CRYPTOCELL, cc310_intr, NULL);
	mdx_intc_set_prio(&dev_nvic, ID_CRYPTOCELL, 0);
	mdx_intc_enable(&dev_nvic, ID_CRYPTOCELL);
}

static void
setup_uicr(void)
{
	uint32_t reg;

	reg = nrf_uicr_read(&dev_uicr, UICR_HFXOSRC);
	if (reg != 0x0e) {
		if (reg != 0xffffffff)
			printf("WARNING: you may need to erase your chip\n");
		nrf_nvmc_write_enable(&dev_nvmc);
		nrf_uicr_write(&dev_uicr, UICR_HFXOSRC, 0xe);
		nrf_nvmc_read_enable(&dev_nvmc);
	}

	reg = nrf_uicr_read(&dev_uicr, UICR_HFXOCNT);
	if (reg != 0x20) {
		if (reg != 0xffffffff)
			printf("WARNING: you may need to erase your chip\n");
		nrf_nvmc_write_enable(&dev_nvmc);
		nrf_uicr_write(&dev_uicr, UICR_HFXOCNT, 0x20);
		nrf_nvmc_read_enable(&dev_nvmc);
	}
}

int
main(void)
{
	uint32_t control_ns;
	uint32_t msp_ns;
	uint32_t psp_ns;
	uint32_t *vec;

	printf("mdepx bootloader started\n");

	malloc_init();

	nrf_power_init(&dev_power, BASE_POWER | PERIPH_SECURE_ACCESS);
	nrf91_errata_init();

	nrf_spu_init(&dev_spu, BASE_SPU);
	arm_nvic_init(&dev_nvic, BASE_NVIC);

	nrf_nvmc_init(&dev_nvmc, BASE_NVMC | PERIPH_SECURE_ACCESS);
	nrf_uicr_init(&dev_uicr, 0xff8000);
	setup_uicr();

	cc310_init();
	secure_boot_configure();

	arm_scb_init(&dev_scb, BASE_SCS_NS);
	arm_scb_set_vector(&dev_scb, APP_ENTRY);
	arm_scb_init(&dev_scb, BASE_SCS);

	vec = (uint32_t *)APP_ENTRY;

	msp_ns = vec[0];
	psp_ns = 0;

	__asm __volatile("msr msp_ns, %0" :: "r" (msp_ns));
	__asm __volatile("msr psp_ns, %0" :: "r" (psp_ns));

	__asm __volatile("mrs %0, control_ns" : "=r" (control_ns));
	control_ns &= ~CONTROL_SPSEL; /* Use main stack pointer. */
	control_ns &= ~CONTROL_NPRIV; /* Set privilege mode. */
	__asm __volatile("msr control_ns, %0" :: "r" (control_ns));

	arm_scb_exceptions_prio_config(&dev_scb, 1);
	arm_scb_exceptions_target_config(&dev_scb, 0);
	arm_scb_sysreset_secure(&dev_scb, 0);
	arm_sau_configure(&dev_scb, 0, 1);
	arm_fpu_non_secure(&dev_scb, 1);

	/* Disable CTRL-AP/NVMC protection needed for development. */
/*
 *	UICR protection bit status		|	NVMC protection
 * SECUREAPPROTECT APPROTECT ERASEPROTECT	| CTRL-AP	NVMC
 *						| ERASEALL	ERASEALL
 * 0		0		0		| Available	Available
 * 1		X		0		| Available	Blocked
 * X		1		0		| Available	Blocked
 * X		X		1		| Blocked	Blocked
 */

	/* ERASEPROTECT is 0 out of reset (default). */

	/* Disable SECUREAPPROTECT (s). */
	*(uint32_t *)0x50039E00 = 0x5A;

	/* Disable APPROTECT (ns). */
	*(uint32_t *)0x40039E10 = 0x5A;

	/* Now look for the app entry point. */
	if (vec[1] == 0xffffffff)
		panic("could not find an app to jump to.");

	printf("Jumping to non-secure address 0x%x\n", vec[1]);

	secure_boot_configure_periph(ID_UARTE0);

	jump_ns(vec[1]);

	/* UNREACHABLE */

	return (0);
}
