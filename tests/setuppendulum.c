#include "../setup.h"

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
   AD1CON2bits.CHPS = 0;	// Selects Channels Utilized bits, When AD12B = 1, CHPS<1:0> is: U-0, Unimplemented, Read as �0�
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
 * Funci�n:	ADC2_init()						   										      *
 * Descripci�n:	Configures ADC2.			 		    								      *
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
   AD2CON2bits.CHPS = 0;	// Bits selecci�n de canal utilizado. Converts CH0
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
	LATDbits.LATD8=0; //Estas dos lineas configuran una salida digital 
	TRISDbits.TRISD8=0;//del puerto D numero 8, pin7 del CON5 

	TRISE = 0x0000;		// Puerto E como salida.

	PTCONbits.PTMOD = 0;	// Modo base de tiempos. (2 -> up&down)
	OVDCON = 0x0000;		// Deshabilitar todas las salidas PWM
	
	PTPER = 0x3fff;		// Periodo para frecuencia deseada
	//200uS=0x0fff
	//150uS=0x0bff
	//25uS=0x01ff
	//14uS(aprox)=0x00ff
	PWMCON1 = 0x0111;		// Habilita salida 1 del PWM y modo independiente 
	PWMCON2bits.IUE = 0;	// Immediate update period enable
	PWMCON2bits.UDIS = 0;	// Actualizaciones de ciclo trabajo y periodo habilitadas

	OVDCON = 0xff00;		// Salidas PWM controladas por el m�dulo PWM
	PDC1 = 0x000;//5F3B;		// Ciclo de trabajo del PWM.  (0x5F3B -> 2.5v)
	 
	PTCONbits.PTEN = 1;		// Empieza PWM.
	
	//Para tocar el PWM:        PDC1 = 0x1000;//2500;//0x2600; Establece el ciclo de trabajo
 	//							LATDbits.LATD8 ^= 1;//Cambia el sentido de giro
}
