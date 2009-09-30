#ifndef _UART_DMA_
#define _UART_DMA_

#include <p33fj256mc710.h>

/******************************************************************************
 *  FILE		 	: uart_dma.h
 *  DESCRIPTION  	: uart(RS-232) configuration using dma
 *  CPU TYPE     	: dsPIC33FJ256MC710
 *  AUTOR	     	: Antonio Camacho Santiago
 *  PROJECT	     	: DPI2007-61527
 *  REVISION HISTORY:
 *			 VERSION: 0.1
 *     		  AUTHOR: Antonio Camacho Santiago
 * 				DATE: 4th August 2008
 * 			COMMENTS: Adapted from Microchip CE214_UART_loopback demo
 *
 *              TODO: Update comments
******************************************************************************/

/*Tested @ 460800bps with MAX233CPP transceiver and modifying oscillator:
	CLKDIVbits.PLLPRE=2;
	CLKDIVbits.PLLPOST=0;
	PLLFBD=146;//148; //Ok for UART1 baudrate=460800 bits/s
	while(OSCCONbits.LOCK!=1);
	Be aware of system time "Tick<>25ns" with the new clock configuration
*/

#define BITRATE1 115200 //921600 //460800 //230400 //115200

#define OUTPUT_DATA_SIZE 24
#define INPUT_DATA_SIZE 28

#define FCY      40000000
#define BRGVAL   ((FCY/BITRATE1)/16)-1







// UART Configuration
void cfgUart1(void);

// DMA4 configuration
void cfgDma4UartTx(void);

// DMA1 configuration
void cfgDma5UartRx(void);



//********************************************************************************
//  STEP 7:
//	Setup DMA interrupt handlers
//	Force transmit after 8 words are received
//********************************************************************************/
void __attribute__((interrupt,no_auto_psv)) _DMA4Interrupt(void);

void __attribute__((interrupt,no_auto_psv)) _DMA5Interrupt(void);


void __attribute__((interrupt,no_auto_psv)) _U1ErrInterrupt(void);

void UART1_DMA_Init();

void Send(char *buffer);

#endif
