
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



struct data{
	unsigned int sem;
	unsigned long d;
	unsigned long t;
};
struct data* d;
struct data* d1;
static double x1=0,x1_ant=0,x2=0,x3=0,x3_ant=0,x4=0;

//las mejores k
static double k1=25,k2=15,k3=32,k4=9;//k1=10,k2=0,k3=22,k4=4; k1=10,k2=0.3,k3=32,k4=9.9;
//static double k1=59.5448,k2=29.8769,k3=70.8934,k4=-6.6124;


//63.2494   28.1537   59.3897   -9.2252
static const double V_MAX=3.3;	
void TaskPWM(void *args)
{
	d=(struct data*)args;
	
	d->t = srtGetRelease();
	d->d = srtGetDeadline();

	while(1){
		//LATBbits.LATB10 ^=1;
		//LATBbits.LATB10 ^=1;
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
			//	//u=-3000;
			u=V_MAX;
			LATDbits.LATD8=0;
		}else if (PulseEncoder_pos<-4500){
			//	//u=3000;
			u=V_MAX;
			LATDbits.LATD8=1;
		}	
		
		
		if(fabs(u)>V_MAX) u=V_MAX;
		
		PDC1=fabs(u*5000/V_MAX)+100;//100 bien
		
		srtSleep(SECONDS2TICKS(0.03), SECONDS2TICKS(0.03));
	}


}

/* Send a character using the UART port
*/
unsigned char buffer[24];
void TaskSend(void *args)
{
	static unsigned char *p_t;
	d1=(struct data*)args;
	
	d1->t = srtGetRelease();
	d1->d = srtGetDeadline();
	
	while(1){
	//LATBbits.LATB10 ^=1;
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

		
		//p_t=&vel_QEncoder;
		//buffer[9]=*p_t;
		//buffer[10]=*(p_t+1);

		
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

inline void delay(unsigned int us){
	unsigned int i;
	for(i=0;i<us;i++){
		asm volatile("do #15, inner1" );	
		asm volatile("nop");
		asm volatile("inner1: nop");
	}
}

void taskLoad1(void *args){

	
	while(1){
		//asm("bset  LATB,#10");
		delay(990);
		//asm("bclr  LATB,#10");
		srtSleep(SECONDS2TICKS(0.01), SECONDS2TICKS(0.01));
		
	}
}
void taskLoad2(void *args){
	unsigned char *p_t;
	
	while(1){
		//asm("bset  LATB,#10");
		LATBbits.LATB10 ^=1;
		//delay(950);
		unsigned long now=srtCurrentTime();
		unsigned long r=srtGetRelease();
		unsigned long d=srtGetDeadline();
		buffer[0]=0x05;
		p_t=&now;
		buffer[1]=*p_t;
		buffer[2]=*(p_t+1);
		buffer[3]=*(p_t+2);
		buffer[4]=*(p_t+3);
		
		p_t=&r;
		buffer[5]=*p_t;
		buffer[6]=*(p_t+1);
		buffer[7]=*(p_t+2);
		buffer[8]=*(p_t+3);
		
		p_t=&d;
		buffer[9]=*p_t;
		buffer[10]=*(p_t+1);
		buffer[11]=*(p_t+2);
		buffer[12]=*(p_t+3);
		
		Send(&buffer);
		srtSleep(SECONDS2TICKS(0.01), SECONDS2TICKS(0.01));
		
	}
}
void taskLoad3(void *args){

	
	while(1){
		//asm("bset  LATB,#10");
		//delay(100);
		//asm("bclr  LATB,#10");
		asm("nop");
		delay(100000);
		srtSleep(SECONDS2TICKS(0.02), SECONDS2TICKS(0.02));
		
	}
}

struct data datos,datos1;
int main(void)
{
	//PulseEncoder_config();//Importantisima la F-->1111
	UART1_DMA_Init();
	//QuadratureEncoder_config();
	//PWM_config();
	
	srtInitKernel(80);
	//srtCreateTask(TaskPWM, 100, SECONDS2TICKS(0.03), SECONDS2TICKS(0.03), &datos);
	//srtCreateTask(TaskSend, 100, SECONDS2TICKS(0.01), SECONDS2TICKS(0.02), &datos1);
	//PDC1=5100;
	srtCreateTask(taskLoad1, 100, SECONDS2TICKS(0.011), SECONDS2TICKS(0.02), 0);
	srtCreateTask(taskLoad1, 100, SECONDS2TICKS(0.012), SECONDS2TICKS(0.02), 0);
	srtCreateTask(taskLoad1, 100, SECONDS2TICKS(0.013), SECONDS2TICKS(0.02), 0);
	srtCreateTask(taskLoad1, 100, SECONDS2TICKS(0.014), SECONDS2TICKS(0.02), 0);
	srtCreateTask(taskLoad1, 100, SECONDS2TICKS(0.015), SECONDS2TICKS(0.02), 0);
	srtCreateTask(taskLoad1, 100, SECONDS2TICKS(0.016), SECONDS2TICKS(0.02), 0);
	srtCreateTask(taskLoad1, 100, SECONDS2TICKS(0.017), SECONDS2TICKS(0.02), 0);
	srtCreateTask(taskLoad1, 100, SECONDS2TICKS(0.018), SECONDS2TICKS(0.02), 0);
	srtCreateTask(taskLoad1, 100, SECONDS2TICKS(0.019), SECONDS2TICKS(0.02), 0);
	/*srtCreateTask(taskLoad2, 100, SECONDS2TICKS(0.019), SECONDS2TICKS(0.2), 0);
	srtCreateTask(taskLoad3, 100, SECONDS2TICKS(1), SECONDS2TICKS(1), 0);*/
	srtCreateTask(taskLoad2, 100, SECONDS2TICKS(0.01), SECONDS2TICKS(0.02), 0);

	/* Forever loop: background activities (if any) should go here */
	for (;;);
	
	return 0;
}
