
#include "../simplertk.h"
#include "../uart_dma.h"
#include "../setup.h"

#include <math.h>
#define PI 3.14159265358979323846
#define STEP 0.0001124148372445117
            							
static signed int PulseEncoder_pos=0;
static unsigned int PortRD4=0;
static unsigned int PortRD5=0;
static unsigned int last_PortRD4=0;
static unsigned int last_PortRD5=0;

static float u=0;
static signed int vel_PEncoder=0;


static signed int pos_QEncoder=0;
static signed int pos_QEncoder_old=0;
static signed int vel_QEncoder=0;
static signed int temp_pos_QEncoder=0;
static int contador=0;
static int temp=0;

static int contador2=0;
static signed int turns=0;
static int Q12=1;
static float KineticEnergy=0;
static float PotentialEnergy=0;
unsigned char RXBuff = 0;
static float aaaa=0;
static int sign=0;
static int contadorx=0;

static unsigned int a;

struct data{
	unsigned int sem;
	unsigned long d;
	unsigned long t;
};
struct data* d;
struct data* d1;
static double x1=0,x1_ant=0,x2=0,x3=0,x3_ant=0,x4=0;
static double k1=10,k2=0,k3=22,k4=4;
static const double V_MAX=3.3;	
void TaskPWM(void *args)
{
	d=(struct data*)args;
	
	d->t = srtGetRelease();
	d->d = srtGetDeadline();

	while(1){
		x1=PulseEncoder_pos*STEP;
		x2=(x1-x1_ant)/0.05;
		
		x3=POSCNT*(2.0*PI/4096.0)-PI;//(-Pi=pendulo abajo)
		x4=(x3-x3_ant)/0.05;
		
		x1_ant=x1;
		x3_ant=x3;
		
		//1250
		if (PulseEncoder_pos>3000){ 
		//	//u=-3000;
			u=V_MAX;
			LATDbits.LATD8=0;
		}else if (PulseEncoder_pos<-3000){
		//	//u=3000;
			u=V_MAX;
			LATDbits.LATD8=1;
		}else{
			u=-k1*x1-k2*x2-k3*x3-k4*x4;
			if(u<0)LATDbits.LATD8=0; //izquierda
			if(u>0)LATDbits.LATD8=1; //derecha
		}		
		
		
		if(fabs(u)>V_MAX) u=V_MAX;
		
		PDC1=fabs(u*5000/V_MAX);//100 bien
		
		d->t+= SECONDS2TICKS(0.05);
		d->d+= SECONDS2TICKS(0.05);
		srtSleepUntil(d->t, d->d);
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

		
		p_t=&vel_QEncoder;
		buffer[9]=*p_t;
		buffer[10]=*(p_t+1);

		
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
		
		d1->t+= SECONDS2TICKS(0.1);
		d1->d+= SECONDS2TICKS(0.1);
		srtSleepUntil(d1->t, d1->d);
		
	}
}

/* Get a character from the UART buffer
 
void TaskReceive()
{
	LATBbits.LATB14 ^= 1;
	
	if (EE_UART1_Receive(&RXBuff) == 0) {
		EE_UART1_Send('<');
		EE_UART1_Send(RXBuff);
		EE_UART1_Send('>');
	}
		
}*/
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



void __attribute__((__interrupt__)) _CNInterrupt(void)
{
IFS1bits.CNIF=0; // Clear interrupt flag

PortRD4=PORTDbits.RD4;
PortRD5=PORTDbits.RD5;

if (PortRD4 != last_PortRD4)PulseEncoder_pos++;
if (PortRD5 != last_PortRD5)PulseEncoder_pos--;

last_PortRD4=PortRD4;
last_PortRD5=PortRD5;
    
}

void PulseEncoder_config()
{
/* set BUTTON pins as inputs */
	TRISDbits.TRISD4  = 1; 
	TRISDbits.TRISD5  = 1; 

	CNEN1bits.CN13IE = 1;
	CNEN1bits.CN14IE = 1;
			
	IFS1bits.CNIF = 0;
	IEC1bits.CNIE = 1;
}



struct data datos,datos1;
int main(void)
{
	PulseEncoder_config();//Importantisima la F-->1111

	QuadratureEncoder_config();
	PWM_config();
	
	srtInitKernel(80);
	srtCreateTask(TaskPWM, 100, SECONDS2TICKS(0.01), SECONDS2TICKS(0.01), &datos);
	srtCreateTask(TaskSend, 100, SECONDS2TICKS(1), SECONDS2TICKS(0.1), &datos1);
	

	/* Program a cyclic alarm which will fire after an offset of 10 counter 
	* ticks, and after that periodically every 500 ticks */
	/*SetRelAlarm(AlarmQuadratureEncoder, 500, 500);
	SetRelAlarm(AlarmPulseEncoder, 500, 500);
	SetRelAlarm(AlarmPWM, 500, 10);
	SetRelAlarm(AlarmSend, 500, 250);
	SetRelAlarm(AlarmReceive, 500, 500);*/
	
	/* Forever loop: background activities (if any) should go here */
	for (;;);
	
	return 0;
}
