/*
 * cli_uart.c - msp430fr5739 experiment board application uart interface 
                implementation
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


//*****************************************************************************
//
//! \addtogroup CLI_UART
//! @{
//
//*****************************************************************************

#include "cli_uart.h"
#include <string.h>
#include <msp430.h>


#define ASCII_ENTER     0x0D

#ifdef _USE_CLI_
unsigned char *g_ucUARTBuffer;

int cli_have_cmd = 0;
#endif

/*!
    \brief          Application Uart interrupt handler

    \param[in]      none

    \return         none

    \note

    \warning
*/
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    switch(__even_in_range(UCA0IV,0x08))
    {
        case 0:break;                             /* Vector 0 - no interrupt */
        case 2:                                   /* Vector 2 - RXIFG */
#ifdef _USE_CLI_
            *g_ucUARTBuffer = UCA0RXBUF;                  
            if (*g_ucUARTBuffer == ASCII_ENTER)
            {
                cli_have_cmd = 1;
                *g_ucUARTBuffer = 0x00;
                __bic_SR_register_on_exit(LPM3_bits);
            }
            g_ucUARTBuffer++;
#endif
            __no_operation();
            break;
        case 4:break;                             /* Vector 4 - TXIFG */
        default: break;  
    }
}

int
CLI_Read(unsigned char *pBuff)
{
    if(pBuff == NULL)
        return -1;
#ifdef _USE_CLI_
    cli_have_cmd = 0;
    g_ucUARTBuffer = pBuff;
    UCA0IE   |= UCRXIE;
    
    __bis_SR_register(LPM2_bits + GIE);
    
    while(cli_have_cmd == 0);
    UCA0IE &= ~UCRXIE;  
    
    return strlen((const char *)pBuff);
#else
    return 0;
#endif
}


int
CLI_Write(unsigned char *inBuff)
{
    if(inBuff == NULL)
        return -1;
#ifdef _USE_CLI_
    unsigned short ret,usLength = strlen((const char *)inBuff);
    ret = usLength;
    while (usLength)
    {
        while (!(UCA0IFG&UCTXIFG));
        UCA0TXBUF = *inBuff;
        usLength--;
        inBuff++;
    }
    return (int)ret;
#else
    return 0;
#endif
}


void
CLI_Configure(void)
{
#ifdef _USE_CLI_
    /* Configure UART pins P2.0 & P2.1 */
    P2SEL1 |= BIT0 + BIT1;
    P2SEL0 &= ~(BIT0 + BIT1);
    
    UCA0CTL1 |= UCSWRST; 
    UCA0CTL1 = UCSSEL_2;                      /* Set SMCLK as UCLk */
    UCA0BRW = 0x9C4;                          /* 9600 baud */
    /* 8000000/(9600*16) - INT(8000000/(9600*16))=0.083 */
    /* UCA0BR1 = 0; */
    /* UCBRFx = 1, UCBRSx = 0x49, UCOS16 = 1 (Refer User Guide) */
    UCA0MCTLW = 0;
                                            
    UCA0CTL1 &= ~UCSWRST;                     /* release from reset */
    UCA0IE   &= ~UCRXIE;
#endif    
}


//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
