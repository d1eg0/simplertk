#include "setup.h"

/******************************************************************************
 * Function:	ADC1_init()
 * Description:	Configures ADC1
 ******************************************************************************/
void ADC1_init(void)
{
   /* (CON6-Pin33)-(AN13-RB13) Pin as input */
   LATBbits.LATB13 = 0;		/* set LED (AN13/RB13) drive state low */
   TRISBbits.TRISB13 = 1;	/* set LED pin (AN13/RB13) as input */

   /*ADC Configuration*/
   AD1PCFGL = 0xFFFF;		//ADC1 Port Configuration Register Low
   AD1PCFGH = 0xFFFF;		//ADC1 Port Configuration Register High

   AD1PCFGLbits.PCFG13 = 0;	// AN13 is Analog Input

   AD1CON2bits.VCFG = 0;    //Converter Voltage Reference Configuration bits (ADRef+=AVdd, ADRef-=AVss)
   AD1CON3bits.ADCS = 63;//3;	// ADC Conversion Clock Select bits (Tad = Tcy*(ADCS+1) = (1/40000000)*64 = 1.6us)
						    //Tcy=Instruction Cycle Time=40MIPS
   AD1CON2bits.CHPS = 0;	// Selects Channels Utilized bits, When AD12B = 1, CHPS<1:0> is: U-0, Unimplemented, Read as ‘0’
   AD1CON1bits.SSRC = 7;	/*Sample Clock Source Select bits:
							  111 = Internal counter ends sampling and starts conversion (auto-convert)
							  110 = Reserved
							  101 = Reserved
							  100 = Reserved
							  011 = MPWM interval ends sampling and starts conversion
							  010 = GP timer (Timer3 for ADC1, Timer5 for ADC2) compare ends sampling and starts conversion
							  001 = Active transition on INTx pin ends sampling and starts conversion
							  000 = Clearing sample bit ends sampling and starts conversion*/

   AD1CON3bits.SAMC = 31;//10;	// Auto Sample Time bits. (31*Tad = 49.6us)
   AD1CON1bits.FORM = 0;	// Data Output Format bits. Integer
							/* For 12-bit operation:
							   11 = Signed fractional (DOUT = sddd dddd dddd 0000, where s = .NOT.d<11>)
							   10 = Fractional (DOUT = dddd dddd dddd 0000)
							   01 = Signed Integer (DOUT = ssss sddd dddd dddd, where s = .NOT.d<11>)
							   00 = Integer (DOUT = 0000 dddd dddd dddd)*/

   AD1CON1bits.AD12B = 1;	/* Operation Mode bit:
							   0 = 10 bit
							   1 = 12 bit*/
   AD1CON1bits.ASAM  = 1;	/* ADC Sample Auto-Start bit:
						       1 = Sampling begins immediately after last conversion. SAMP bit is auto-set.
							   0 = Sampling begins when SAMP bit is set*/
   AD1CHS0bits.CH0SA = 13;	// MUXA +Ve input selection (AIN13) for CH0.
   AD1CHS0bits.CH0NA = 0;	// MUXA  -Ve input selection (Vref-) for CH0.

   AD1CON1bits.ADON  = 1;	// ADC Operating Mode bit. Turn on the A/D converter


}

/******************************************************************************************
 * Función:	ADC2_init()						   										      *
 * Descripción:	Configures ADC2.			 		    								      *
 ******************************************************************************************/
void ADC2_init(void)
{
   /* (CON6-Pin34)-(AN12-RB12) Pin as input */
   LATBbits.LATB12 = 0;		/* set LED (AN12/RB12) drive state low */
   TRISBbits.TRISB12 = 1;	/* set LED pin (AN12/RB12) as input */

   /*ADC Configuration*/
   AD2PCFGL = 0xFFFF; //ADC2 Port Configuration Register Low
  // AD2PCFGH = 0xFFFF; //ADC2 Port Configuration Register High

   AD2PCFGLbits.PCFG12 = 0;	// AN12 is Analog Input

   AD2CON2bits.VCFG = 0;    //Converter Voltage Reference Configuration bits (ADRef+=AVdd, ADRef-=AVss)
   AD2CON3bits.ADCS = 63;//3;	// ADC Conversion Clock Select bits (Tad = Tcy*(ADCS+1) = (1/40000000)*64 = 1.6us)
						    //Tcy=Instruction Cycle Time=40MIPS
   AD2CON2bits.CHPS = 0;	// Bits selección de canal utilizado. Converts CH0
   AD2CON1bits.SSRC = 7;	/*Sample Clock Source Select bits:
							  111 = Internal counter ends sampling and starts conversion (auto-convert)
							  110 = Reserved
							  101 = Reserved
							  100 = Reserved
							  011 = MPWM interval ends sampling and starts conversion
							  010 = GP timer (Timer3 for ADC1, Timer5 for ADC2) compare ends sampling and starts conversion
							  001 = Active transition on INTx pin ends sampling and starts conversion
							  000 = Clearing sample bit ends sampling and starts conversion*/
   AD2CON3bits.SAMC = 31;//10;	// Auto Sample Time bits. (31*Tad = 49.6us)
   AD2CON1bits.FORM = 0;	// Data Output Format bits. Integer
							/* For 12-bit operation:
							   11 = Signed fractional (DOUT = sddd dddd dddd 0000, where s = .NOT.d<11>)
							   10 = Fractional (DOUT = dddd dddd dddd 0000)
							   01 = Signed Integer (DOUT = ssss sddd dddd dddd, where s = .NOT.d<11>)
							   00 = Integer (DOUT = 0000 dddd dddd dddd)*/

   AD2CON1bits.AD12B = 1;	/* Operation Mode bit:
							   0 = 10 bit
							   1 = 12 bit*/
   AD2CON1bits.ASAM  = 1;	/* ADC Sample Auto-Start bit:
						       1 = Sampling begins immediately after last conversion. SAMP bit is auto-set.
							   0 = Sampling begins when SAMP bit is set*/
   AD2CHS0bits.CH0SA = 12;	// MUXA +Ve input selection (AIN12) for CH0.
   AD2CHS0bits.CH0NA = 0;	// MUXA  -Ve input selection (Vref-) for CH0.

   AD2CON1bits.ADON  = 1;	// ADC Operating Mode bit. Turn on the A/D converter
}


/******************************************************************************
 * Function:	PWM_config()
 * Description:	Configures PWM actuator
 ******************************************************************************/
void PWM_config(void)
{
//	TRISE = 0x0000;			// Port E as output

	PTCONbits.PTMOD = 0;	/*PWM Time Base Mode Select bits
							  11 =PWM time base operates in a Continuous Up/Down Count mode with interrupts for double PWM updates
							  10 =PWM time base operates in a Continuous Up/Down Count mode
							  01 =PWM time base operates in Single Pulse mode
							  00 =PWM time base operates in a Free-Running mode*/

//	OVDCON = 0x0000;		// Override Control Register

	PTPER = 0x3FFF;			// PWM Time Base Period Value bits, PWM period selection
	PWMCON1 = 0x0111;		// PWM Control Register 1

	PWMCON2bits.IUE = 0;	// Immediate update period enable
	PWMCON2bits.UDIS = 0;	/* PWM Update Disable bit
							   1 = Updates from Duty Cycle and Period Buffer registers are disabled
							   0 = Updates from Duty Cycle and Period Buffer registers are enabled*/

	OVDCON = 0xff00;		// Override Control Register
	PDC1 = 0x2550;			// Initial PWM value

	PTCONbits.PTEN = 1;		// Enable PWM.
}
