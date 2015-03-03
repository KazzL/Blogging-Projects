/*
 * spi.c - msp430f5529 launchpad spi interface implementation
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

#ifndef SL_IF_TYPE_UART
#include <msp430.h>

#include "simplelink.h"
#include "spi.h"
#include "board.h"

#define ASSERT_CS()          (P2OUT &= ~BIT2)
#define DEASSERT_CS()        (P2OUT |= BIT2)

//Other SPI Devices (1 - 3)
#define ASSERT_CS_1()          (P2OUT &= ~BIT3)
#define DEASSERT_CS_1()        (P2OUT |= BIT3)
#define ASSERT_CS_2()          (P2OUT &= ~BIT4)
#define DEASSERT_CS_2()        (P2OUT |= BIT4)
#define ASSERT_CS_3()          (P2OUT &= ~BIT5)
#define DEASSERT_CS_3()        (P2OUT |= BIT5)

int spi_Close(Fd_t fd)
{
    /* Disable WLAN Interrupt ... */
    CC3100_InterruptDisable();

    return NONOS_RET_OK;
}

Fd_t spi_Open(char *ifName, unsigned long flags)
{
    /* Select the SPI lines: MOSI/MISO on P3.0,1 CLK on P3.2 */
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


    /* P1.6 - WLAN enable full DS */
    P1SEL &= ~BIT6;
    P1OUT &= ~BIT6;
    P1DIR |= BIT6;


    /* Configure SPI IRQ line on P2.0 */
    P2DIR &= ~BIT0;
    P2SEL &= ~BIT0;

    P2REN |= BIT0;

    /* Configure the SPI CS to be on P2.2 */
    P2OUT |= BIT2;
    P2SEL &= ~BIT2;
    P2DIR |= BIT2;

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

    return len_to_return; //TODO: Mention this as incorrect return, will not inform as API specifies.
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








int spi_Init(void)
{
    /* Select the SPI lines: MOSI/MISO on P3.3,4 CLK on P2.7 */
    P3SEL |= (BIT3 + BIT4);

//    P3REN |= BIT4;
//    P3OUT |= BIT4;

    /* Select the SPI clock on P2.7*/
    P2SEL |= BIT7;
    P2DIR |= BIT7;

    UCA0CTL1 |= UCSWRST; /* Put state machine in reset */
    UCA0CTL0 = UCMSB + UCMST + UCSYNC;// + UCCKPH; /* 3-pin, 8-bit SPI master */

    UCA0CTL1 = UCSWRST + UCSSEL_2; /* Use SMCLK, keep RESET */

    /* Set SPI clock */
    UCA0BR0 = 0x05; /* f_UCxCLK = 25MHz/5 */
    UCA0BR1 = 0;
    UCA0CTL1 &= ~UCSWRST;

    /* Configure SPI Device 1 CS to be on P2.3 */
    P2OUT |= BIT3;
    P2SEL &= ~BIT3;
    P2DIR |= BIT3;
    /* Configure SPI Device 2 CS to be on P2.4 */
    P2OUT |= BIT4;
    P2SEL &= ~BIT4;
    P2DIR |= BIT4;
    /* Configure SPI Device 3 CS to be on P2.5 */
    P2OUT |= BIT5;
    P2SEL &= ~BIT5;
    P2DIR |= BIT5;

    /* 50 ms delay */
    Delay(50);

    return 1;
}

int spi_Device_Write(unsigned char *pBuff, int len, int deviceNumber)
{
    switch(deviceNumber)
    {
		case 1:
			ASSERT_CS_1();
			break;
		case 2:
			ASSERT_CS_2();
			break;
		case 3:
			ASSERT_CS_3();
			break;
		default:
			len = -1;
			break;
    }
    while (len)
    {
        while (!(UCA0IFG & UCTXIFG));
        UCA0TXBUF = *pBuff;
        while (!(UCA0IFG & UCRXIFG));
        UCA0RXBUF;
        len --;
        pBuff++;
    }

    switch(deviceNumber)
    {
        case 1:
        	DEASSERT_CS_1();
        	break;
        case 2:
        	DEASSERT_CS_2();
        	break;
        case 3:
        	DEASSERT_CS_3();
        	break;
        default:
        	len = -1;
        	break;
    }

    return len;
}


int spi_Device_Read(unsigned char *pBuff, int len, int deviceNumber)
{
    int i = 0;

    switch(deviceNumber)
    {
		case 1:
			ASSERT_CS_1();
			break;
		case 2:
			ASSERT_CS_2();
			break;
		case 3:
			ASSERT_CS_3();
			break;
		default:
			len = -1;
			break;
    }

    for (i = 0; i < len; i ++)
    {
        while (!(UCA0IFG & UCTXIFG));
        UCA0TXBUF = 0xFF;
        while (!(UCA0IFG & UCRXIFG));
        pBuff[i] = UCA0RXBUF;
    }

    switch(deviceNumber)
    {
		case 1:
			DEASSERT_CS_1();
			break;
		case 2:
			DEASSERT_CS_2();
			break;
		case 3:
			DEASSERT_CS_3();
			break;
		default:
			len = -1;
			break;
    }

    return len;
}


#endif /* SL_IF_TYPE_UART */
