#include "uart_dma.h"

//********************************************************************************
//  Allocate two buffers for DMA transfers
//********************************************************************************/
/** Output Buffer */
char BufferOutput[OUTPUT_DATA_SIZE] __attribute__((space(dma)));
char BufferInput[INPUT_DATA_SIZE] __attribute__((space(dma)));

// UART Configuration
void cfgUart1(void)
{
	U1MODEbits.STSEL = 0;			// 1-stop bit
	U1MODEbits.PDSEL = 0;			// No Parity, 8-data bits
	U1MODEbits.ABAUD = 0;			// Autobaud Disabled

	U1BRG = BRGVAL;					// BAUD Rate Setting ((FCY/BITRATE1)/16)-1


	//********************************************************************************
	//  STEP 1:
	//  Configure UART for DMA transfers
	//********************************************************************************/
	U1STAbits.UTXISEL0 = 0;			// Interrupt after one Tx character is transmitted
	U1STAbits.UTXISEL1 = 0;
	U1STAbits.URXISEL  = 0;			// Interrupt after one RX character is received


	//********************************************************************************
	//  STEP 2:
	//  Enable UART Rx and Tx
	//********************************************************************************/
	U1MODEbits.UARTEN   = 1;		// Enable UART
	U1STAbits.UTXEN     = 1;		// Enable UART Tx


	IEC4bits.U1EIE = 0;
}

// DMA4 configuration
void cfgDma4UartTx(void)
{
	//********************************************************************************
	//  STEP 3:
	//  Associate DMA Channel 0 with UART Tx
	//********************************************************************************/
	DMA4REQ = 0x000C;//0x001F;					// Select UART1 Transmitter
	DMA4PAD = (volatile unsigned int) &U1TXREG;

	//********************************************************************************
	//  STEP 5:
	//  Configure DMA Channel 0 to:
	//  Transfer data from RAM to UART
	//  One-Shot mode
	//  Register Indirect with Post-Increment
	//  Using single buffer
	//  8 transfers per buffer
	//  Transfer words
	//********************************************************************************/
	//DMA0CON = 0x2001;					// One-Shot, Post-Increment, RAM-to-Peripheral

	DMA4CONbits.AMODE = 0;
	DMA4CONbits.MODE  = 1;
	DMA4CONbits.DIR   = 1;
	DMA4CONbits.SIZE  = 1;//0;
	DMA4CNT = OUTPUT_DATA_SIZE-1;//18;//7;						// 8 DMA requests


	//********************************************************************************
	//  STEP 6:
	// Associate one buffer with Channel 0 for one-shot operation
	//********************************************************************************/
	DMA4STA = __builtin_dmaoffset(BufferOutput);

	//********************************************************************************
	//  STEP 8:
	//	Enable DMA Interrupts
	//********************************************************************************
	IFS2bits.DMA4IF  = 0;			// Clear DMA Interrupt Flag
	IEC2bits.DMA4IE  = 1;			// Enable DMA interrupt
}

// DMA1 configuration
void cfgDma5UartRx(void)
{
	//********************************************************************************
	//  STEP 3:
	//  Associate DMA Channel 1 with UART Rx
	//********************************************************************************/
	DMA5REQ = 0x000B;				// Select UART1 Receiver
	DMA5PAD = (volatile unsigned int) &U1RXREG;

	//********************************************************************************
	//  STEP 4:
	//  Configure DMA Channel 1 to:
	//  Transfer data from UART to RAM Continuously
	//  Register Indirect with Post-Increment
	//  Using two ‘ping-pong’ buffers
	//  8 transfers per buffer
	//  Transfer words
	//********************************************************************************/
	//DMA1CON = 0x0002;					// Continuous, Ping-Pong, Post-Inc, Periph-RAM

	DMA5CONbits.AMODE = 0;
	DMA5CONbits.MODE  = 1;//2;//2;One-Shot-->0 ping-pong-->2
	DMA5CONbits.DIR   = 0;
	DMA5CONbits.SIZE  = 1;
	DMA5CNT = 14;//INPUT_DATA_SIZE-1;//18;//7;						// 8 DMA requests




	//********************************************************************************
	//  STEP 6:
	//  Associate two buffers with Channel 1 for ‘Ping-Pong’ operation
	//********************************************************************************/
	//DMA5STA = __builtin_dmaoffset(BufferA);
	//DMA5STB = __builtin_dmaoffset(BufferB);
	DMA5STA = __builtin_dmaoffset(BufferInput);
	DMA5STB = __builtin_dmaoffset(BufferInput);

	//********************************************************************************
	//  STEP 8:
	//	Enable DMA Interrupts
	//********************************************************************************/
	IFS3bits.DMA5IF  = 0;			// Clear DMA interrupt
	IEC3bits.DMA5IE  = 1;			// Enable DMA interrupt


	//********************************************************************************
	//  STEP 9:
	//  Enable DMA Channel 1 to receive UART data
	//********************************************************************************/
	DMA5CONbits.CHEN = 1;			// Enable DMA Channel
}



//********************************************************************************
//  STEP 7:
//	Setup DMA interrupt handlers
//	Force transmit after 8 words are received
//********************************************************************************/
void __attribute__((interrupt,no_auto_psv)) _DMA4Interrupt(void)
{
	IFS2bits.DMA4IF = 0;			// Clear the DMA0 Interrupt Flag;
}

void __attribute__((interrupt,no_auto_psv)) _DMA5Interrupt(void)
{
	static unsigned int BufferCount = 0;  // Keep record of which buffer contains Rx Data

//	if(BufferCount == 0)
//	{
//		DMA4STA = __builtin_dmaoffset(BufferA); // Point DMA 0 to data to be transmitted
//	}
//	else
//	{
//		DMA4STA = __builtin_dmaoffset(BufferB); // Point DMA 0 to data to be transmitted
//	}


	//DMA4CONbits.CHEN  = 1;			// Re-enable DMA0 Channel
	//DMA4REQbits.FORCE = 1;			// Manual mode: Kick-start the first transfer


	BufferCount ^= 1;

	IFS3bits.DMA5IF = 0;			// Clear the DMA1 Interrupt Flag
}


void __attribute__((interrupt,no_auto_psv)) _U1ErrInterrupt(void)//ISR2(_U1ErrInterrupt)
{
	IFS4bits.U1EIF = 0; // Clear the UART1 Error Interrupt Flag
}

void UART1_DMA_Init()
{
	cfgDma4UartTx();//This routine Configures DMAchannel 4 for transmission.
	cfgDma5UartRx();//This routine Configures DMAchannel 5 for reception.
	cfgUart1();// UART1 Configurations
}

void Send(char *buffer)
{
    BufferOutput[0]=*buffer;//More significant 8bits of unsigned int (16bits)
    BufferOutput[1]=*(buffer+1);        //Less significant 8bits of unsigned int (16bits)
    BufferOutput[2]=*(buffer+2);//4th byte of float (32bits)
    BufferOutput[3]=*(buffer+3);//3th byte of float (32bits)
    BufferOutput[4]=*(buffer+4);//2nd byte of float (32bits)
    BufferOutput[5]=*(buffer+5);//1st byte of float (32bits)
    BufferOutput[6]=*(buffer+6);
    BufferOutput[7]=*(buffer+7);
    BufferOutput[8]=*(buffer+8);
	BufferOutput[9]=*(buffer+9);
    BufferOutput[10]=*(buffer+10);
    BufferOutput[11]=*(buffer+11);
    BufferOutput[12]=*(buffer+12);
    BufferOutput[13]=*(buffer+13);
    BufferOutput[14]=*(buffer+14);
    BufferOutput[15]=*(buffer+15);
    BufferOutput[16]=*(buffer+16);
    BufferOutput[17]=*(buffer+17);
    BufferOutput[18]=*(buffer+18);
    BufferOutput[19]=*(buffer+19);
    BufferOutput[20]=*(buffer+20);
    BufferOutput[21]=*(buffer+21);
    BufferOutput[22]=*(buffer+22);
	BufferOutput[23]=*(buffer+23);
	BufferOutput[24]=*(buffer+24);



    //Force sending data
    DMA4CONbits.CHEN  = 1;            // Re-enable DMA4 Channel
    DMA4REQbits.FORCE = 1;            // Manual mode: Kick-start the first transfer
}
