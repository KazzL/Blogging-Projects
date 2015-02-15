/*
 * spi.c - msp430fr5739 experiment board spi interface implementation
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


#define ASSERT_CS()          (P1OUT &= ~BIT3)
#define DEASSERT_CS()        (P1OUT |= BIT3)


int spi_Close(Fd_t fd)
{
    /* Disable WLAN Interrupt ... */
    CC3100_InterruptDisable();

    return NONOS_RET_OK;
}

Fd_t spi_Open(char *ifName, unsigned long flags)
{
    /* Select the SPI lines: MISO/MOSI on P1.6,7 CLK on P2.2 */
    P1SEL1 |= (BIT6 + BIT7);
    P1SEL0 &= (~(BIT6 + BIT7));

    P2SEL1 |= (BIT2);
    P2SEL0 &= ~BIT2;

    UCB0CTLW0 |= UCSWRST;                     /* Put state machine in reset */
    UCB0CTLW0 |= (UCMST+UCSYNC+UCMSB+UCCKPH  ); /* 3-pin, 8-bit SPI master */
                                              /* Clock polarity high, MSB */
    UCB0CTLW0 |= UCSSEL_2;                    /* ACLK */

    /* Set SPI clock 12MHz */
    UCB0BR0 = 0x02;
    UCB0BR1 = 0;

    UCB0CTLW0 &= ~UCSWRST;               /* Initialize USCI state machine */

    /* P1.2 - WLAN enable full DS */
    P1SEL1 &= ~BIT2;
    P1SEL0 &= ~BIT2;
    P1OUT &= ~BIT2;
    P1DIR |= BIT2;

    /* Configure SPI IRQ line on P2.3 */
    P2DIR  &= (~BIT3);
    P2SEL1 &= ~BIT3;
    P2SEL0 &= ~BIT3;

    /* Configure the SPI CS to be on P1.3 */
    P1OUT |= BIT3;
    P1DIR |= BIT3;
    P1SEL1 &= ~BIT3;
    P1SEL0 &= ~BIT3;

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
