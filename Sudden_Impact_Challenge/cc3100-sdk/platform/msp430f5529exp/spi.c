/*
 * spi.c - msp430f5529 experiment board spi interface implementation
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#include "simplelink.h"
#include "spi.h"
#include "board.h"
#include <msp430.h>


#define ASSERT_CS()          (P2OUT &= ~BIT6)
#define DEASSERT_CS()        (P2OUT |= BIT6)


int spi_Close(Fd_t fd)
{
    /* Disable WLAN Interrupt ... */
    CC3100_InterruptDisable();

    return NONOS_RET_OK;
}

Fd_t spi_Open(char *ifName, unsigned long flags)
{
    /* Select the SPI lines: MISO/MOSI on P3.0,1 CLK on P3.2 */
    P3SEL |= (BIT0 + BIT1);

    P3REN |= BIT1;
    P3OUT |= BIT1;

    P3SEL |= BIT2;

    UCB0CTL1 |= UCSWRST; /* Put state machine in reset */
    UCB0CTL0 = UCMSB + UCMST + UCSYNC + UCCKPH; /* 3-pin, 8-bit SPI master */

    UCB0CTL1 = UCSWRST + UCSSEL_2; /* Use SMCLK, keep RESET */

    /* Set SPI clock */
    UCB0BR0 = 0x02; /* f_UCxCLK = 25MHz/2 */
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;


    /* P4.6 - WLAN enable full DS */
    P4SEL &= ~BIT6;
    P4OUT &= ~BIT6;
    P4DIR |= BIT6;

    /* Configure SPI IRQ line on P2.4 */
    P2DIR &= ~BIT4;
    P2SEL &= ~BIT4;

    P2REN |= BIT4;

    /* Configure the SPI CS to be on P2.6 */
    P2OUT |= BIT6;
    P2SEL &= ~BIT6;
    P2DIR |= BIT6;

    /* 50 ms delay */
    Delay(50);

    /* Enable WLAN interrupt */
    CC3100_InterruptEnable();

    return NONOS_RET_OK;
}

int spi_Write(Fd_t fd, unsigned char *pBuff, int len)
{
    int len_to_return = len;

    ASSERT_CS();

    while (len)
    {
        while (!(UCB0IFG&UCTXIFG));
        UCB0TXBUF = *pBuff;
        while (!(UCB0IFG&UCRXIFG));
        UCB0RXBUF;
        len --;
        pBuff++;
    }

    DEASSERT_CS();

    return len_to_return;
}

int spi_Read(Fd_t fd, unsigned char *pBuff, int len)
{
    int i = 0;

    ASSERT_CS();

    for (i = 0; i < len; i ++)
    {
        while (!(UCB0IFG&UCTXIFG));
        UCB0TXBUF = 0xFF;
        while (!(UCB0IFG&UCRXIFG));
        pBuff[i] = UCB0RXBUF;
    }

    DEASSERT_CS();

    return len;
}
