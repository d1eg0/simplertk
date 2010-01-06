#include "../simplertk.h"
#include "../uart_dma.h"
#include "../setup.h"

#include <math.h>
#define PI 3.14159265358979323846
#define STEP 0.0001124148372445117

volatile static signed int PulseEncoder_pos=0;
volatile static unsigned int PortRD4=0;
volatile static unsigned int PortRD5=0;
static unsigned int last_PortRD4=0;
static unsigned int last_PortRD5=0;

static float u=0;

static double x1=0,x1_ant=0,x2=0,x3=0,x3_ant=0,x4=0;

// ganancias
static double k1=25,k2=15,k3=32,k4=9;



static const double V_MAX=3.3;	

void TaskPWM(void *args)
{

	while(1){

		x1=PulseEncoder_pos*STEP;
		x2=(x1-x1_ant)/0.05;
		
		x3=POSCNT*(2.0*PI/4096.0)-PI;//(-Pi=pendulo abajo)
		x4=(x3-x3_ant)/0.05;
		
		x1_ant=x1;
		x3_ant=x3;
		
		//1250
		if(PulseEncoder_pos<4500 && PulseEncoder_pos>-4500){
			u=-k1*x1-k2*x2-k3*x3-k4*x4;
			if(u<0)LATDbits.LATD8=0; //izquierda
			if(u>0)LATDbits.LATD8=1; //derecha
		}else if (PulseEncoder_pos>4500){ 
			u=V_MAX;
			LATDbits.LATD8=0;
		}else if (PulseEncoder_pos<-4500){
			u=V_MAX;
			LATDbits.LATD8=1;
		}	
		
		
		if(fabs(u)>V_MAX) u=V_MAX;
		
		PDC1=fabs(u*5000/V_MAX)+100;
		
		srtSleep(SECONDS2TICKS(0.03), SECONDS2TICKS(0.03));
	}


}


unsigned char buffer[24];
void TaskSend(void *args)
{
	static unsigned char *p_t;
	
	while(1){
		unsigned long now=srtCurrentTime();
		unsigned int posq=POSCNT;
		buffer[0]=0x05;
		p_t=&now;
		buffer[1]=*p_t;
		buffer[2]=*(p_t+1);
		buffer[3]=*(p_t+2);
		buffer[4]=*(p_t+3);

		p_t=&posq;
		buffer[5]=*p_t;
		buffer[6]=*(p_t+1);

		
		p_t=&PulseEncoder_pos;
		buffer[7]=*p_t;
		buffer[8]=*(p_t+1);

		
		p_t=&u;
		buffer[11]=*p_t;
		buffer[12]=*(p_t+1);
		buffer[13]=*(p_t+2);
		buffer[14]=*(p_t+3);

		
		p_t=&PDC1;
		buffer[15]=*p_t;
		buffer[16]=*(p_t+1);
		buffer[23]=6;

		
		Send(&buffer);
		
		srtSleep(SECONDS2TICKS(0.01), SECONDS2TICKS(0.01));
		
	}
}


void QuadratureEncoder_config(void)
{
	
	AD1PCFGL=0xFFFF;
	AD2PCFGL=0xFFFF;
	TRISB=0xFFFF;


	POSCNT = 0;
	QEICONbits.QEIM = 0b111; // resolución 4x, modo de Reset por MAXCNT
	QEICONbits.QEISIDL = 0; // continuar en modo idle (0)
	QEICONbits.SWPAB = 1; // Phase-A y Phase-B intercambiados
	QEICONbits.PCDOUT = 1; // activado el pin de Position Counter Status
	QEICONbits.TQGATE = 0; // Timer gate apagado
	QEICONbits.TQCKPS = 0; // Prescaler 1:1
	QEICONbits.POSRES = 0; // Un pulso en INDEX no hace un reset
	QEICONbits.TQCS =0; // Usamos clock interno para el timer
	QEICONbits.UPDN_SRC=1; // Phase-B indica dirección
	MAXCNT=4095;//4*1028=4096 (Se inicializa POSCNT a cero y se pierde el ultimo valor 4096) 1024 pulsos en modo x4 (Mas resolucion cuenta flancos de subida y bajada?)
	POSCNT = 2047;//Arriba=0, Abajo=2047
	DFLTCON = 0x00F0;//Filtro, aun sin determinar
}



void __attribute__((__interrupt__,__auto_psv__)) _CNInterrupt(void)
{
	IFS1bits.CNIF=0; // Clear interrupt flag
	LATBbits.LATB10 =1;
	PortRD4=PORTDbits.RD4;
	PortRD5=PORTDbits.RD5;

	if (PortRD4 != last_PortRD4)PulseEncoder_pos++;
	if (PortRD5 != last_PortRD5)PulseEncoder_pos--;

	last_PortRD4=PortRD4;
	last_PortRD5=PortRD5;
	LATBbits.LATB10 =0;
	
}

void PulseEncoder_config()
{
	/* set BUTTON pins as inputs */
	TRISDbits.TRISD4  = 1; 
	TRISDbits.TRISD5  = 1; 

	CNEN1bits.CN13IE = 1;
	CNEN1bits.CN14IE = 1;
	
	IPC4bits.CNIP = 7;
	IFS1bits.CNIF = 0;
	IEC1bits.CNIE = 1;
	
}



int main(void)
{
	PulseEncoder_config();
	UART1_DMA_Init();
	QuadratureEncoder_config();
	PWM_config();
	
	srtInitKernel(80);
	srtCreateTask(TaskPWM, 100, SECONDS2TICKS(0.03), SECONDS2TICKS(0.03), 0);
	srtCreateTask(TaskSend, 100, SECONDS2TICKS(0.01), SECONDS2TICKS(0.02), 0);

	for (;;);

}
