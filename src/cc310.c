/*-
 * Copyright (c) 2020 Ruslan Bukin <br@bsdpad.com>
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

#include <arm/arm/nvic.h>
#include <arm/nordicsemi/nrf9160.h>

#include "nrf_cc310/include/sns_silib.h"
#include "cc310.h"

static CRYS_RND_State_t		rndState;
static CRYS_RND_WorkBuff_t	rndWorkBuff;

extern struct arm_nvic_softc nvic_sc;
extern struct nrf_spu_softc spu_sc;

void CRYPTOCELL_IRQHandler(void);

static void
cc310_intr(void *arg, struct trapframe *tf, int irq)
{

	CRYPTOCELL_IRQHandler();
}

int
cc310_get_random(uint8_t *out, int size)
{
	int err;

	if (size > (CRYS_RND_SEED_MAX_SIZE_WORDS * 4))
		return (0);

	/* TODO: check out pointer, so it points to non-secure area. */

	err = CRYS_RND_GenerateVector(&rndState, size, (uint8_t *)out);
	if (err)
		return (0);

	return (1);
}

int
cc310_init(void)
{
	uint32_t reg;
	int err;

	reg = BASE_CRYPTOCELL + CRYPTOCELL_ENABLE;
	*(volatile uint32_t *)reg = 1;

	arm_nvic_setup_intr(&nvic_sc, ID_CRYPTOCELL, cc310_intr, NULL);
	arm_nvic_set_prio(&nvic_sc, ID_CRYPTOCELL, 0);
	arm_nvic_enable_intr(&nvic_sc, ID_CRYPTOCELL);

	err = SaSi_LibInit();
	if (err != SA_SILIB_RET_OK) {
		printf("Failed to init SaSi lib\n");
		return (-1);
	}

	err = CRYS_RndInit(&rndState, &rndWorkBuff);
	if (err) {
		printf("Failed to CRYS_RndInit() with err %d\n", err);
		return (-1);
	}

	/*
	 * Reseeding every time we want a random number is inefficient.
	 * The idea is that the RNG is seeded initially, and then reseeded
	 * as needed. Between this we get a pseudo random sequence (PRBS),
	 * but this has very high quality.
	 */

	err = CRYS_RND_Reseeding(&rndState, &rndWorkBuff);
	if (err)
		return (0);

#if 0
	uint8_t rand[48];
	while (1) {
		err = cc310_get_random(rand, 48);
		printf("err %d, rand %x\n", err, *(uint32_t *)rand);
	}
#endif

	return (0);
}
