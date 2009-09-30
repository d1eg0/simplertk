#include "simplertk.h"
#include "uart_dma.h"


 /*	Constants	*/
static float const v_max=3.33;  //dsPIC voltage reference
static float const Nu =0;
static float const Nx[2] ={1.0,0};

/*	Variables	*/
static float read_adc1=0;
static float read_adc2=0;

static float reference=0.5;
static float r=0.5;

static float x[2]={0,0};
static float x_hat[2]={0,0};

static float u=0;
static float u_ss=0;


/* Controller gains, u=-k*x */
static float k[2]={0.5029,-0.9519};


unsigned char buffer[23];
struct data{
	unsigned int sem;
	unsigned long d;
	unsigned long t;
};
struct data* d;
struct data* d2;
struct data* d3;
struct data* d4;



/* Read capacitors voltages */
void Read_State(void)
{
	while (AD1CON1bits.DONE == 0);	// Waits until ADC1 finish
	read_adc1 = ADC1BUF0;			// Read ADC1 Buffer

	while (AD2CON1bits.DONE == 0);	// Waits until ADC2 finish
	read_adc2 = ADC2BUF0;			// Read ADC2 Buffer

	x[0]=(read_adc1/4096)*v_max-(v_max/2.0);
	x[1]=(read_adc2/4096)*v_max-(v_max/2.0)-0.02;
}

void taskreference(void *args){
	d2=(struct data*)args;
	
	d2->t = srtGetRelease();
	d2->d = srtGetDeadline();
	while(1){
		
		if (reference == -0.5)
		{
			reference=0.5;
			LATBbits.LATB14 = 1;//Orange led switch on
		}else{
			reference=-0.5;
			LATBbits.LATB14 = 0;//Orange led switch off
		}
		//reference=0.5;
		/*if (d2->sem==0){
			//LATBbits.LATB14 = 1;
			//LATDbits.LATD0=1;
			//LATFbits.LATF1=1;
			d2->sem=1;
		}else{
			//LATBbits.LATB14 = 0;
			//LATDbits.LATD0=0;
			//LATFbits.LATF1=0;
			d2->sem=0;
		}*/
		d2->t+= SECONDS2TICKS(1);
		d2->d+= SECONDS2TICKS(1);
		srtSleepUntil(d2->t, d2->d);
	}	
}

void taskcontroller(void *args){
	d3=(struct data*)args;
	
	d3->t = srtGetRelease();
	d3->d = srtGetDeadline();
	while(1){

		r=reference;

		Read_State();

		x_hat[0]=x[0]-r*Nx[0];
		x_hat[1]=x[1]-r*Nx[1];
		u_ss=r*Nu;

		u=-k[0]*x_hat[0]-k[1]*x_hat[1]+u_ss;

		/* Output saturation */
		if (u>v_max/2) u=v_max/2;
		if (u<-v_max/2) u=-v_max/2;

		PDC1= u*0x7FFF/v_max+16600;//16384;//Scale to PWM frequency
			
		d3->t+= SECONDS2TICKS(0.05);
		d3->d+= SECONDS2TICKS(0.05);
		srtSleepUntil(d3->t, d3->d);
	}	
}

void tasksend(void *args){

	static unsigned char *p_r= &r;
	static unsigned char *p_x0= &x[0];
	static unsigned char *p_x1= &x[1];
	static unsigned char *p_u= &u;
	static unsigned char *p_t;
	//LATBbits.LATB10 = 1; //To get time with the oscilloscope

	//Read_State();
	//static unsigned char *ptr,*ptrn,*ptrd;
	d=(struct data*)args;
	
	d->t = srtGetRelease();
	d->d = srtGetDeadline();
	while(1){
						

		//LATBbits.LATB10 ^=1;

		Read_State();
		unsigned long now=srtCurrentTime();
		p_t=&now;
		p_r=&r;
		p_x0=&x[0];
		p_x1=&x[1];
		p_u=&u;
		
		buffer[0]=0x05;
		buffer[1]=*p_r;
		buffer[2]=*(p_r+1);
		buffer[3]=*(p_r+2);
		buffer[4]=*(p_r+3);
		buffer[5]=*p_x0;
		buffer[6]=*(p_x0+1);
		buffer[7]=*(p_x0+2);
		buffer[8]=*(p_x0+3);
		buffer[9]=*p_x1;
		buffer[10]=*(p_x1+1);
		buffer[11]=*(p_x1+2);
		buffer[12]=*(p_x1+3);
		buffer[13]=*p_u;
		buffer[14]=*(p_u+1);
		buffer[15]=*(p_u+2);
		buffer[16]=*(p_u+3);
		buffer[17]=*p_t;
		buffer[18]=*(p_t+1);
		buffer[19]=*(p_t+2);
		buffer[20]=*(p_t+3);
		buffer[21]=1;
		buffer[22]=2;
		buffer[23]=3;
		buffer[24]=4;

		
		Send(&buffer);
		 //Force sending data
		//DMA4CONbits.CHEN  = 1;            // Re-enable DMA4 Channel
		//DMA4REQbits.FORCE = 1;            // Manual mode: Kick-start the first transfer

		
		//LATBbits.LATB14 = 1;
		d->t+= SECONDS2TICKS(0.01);
		d->d+= SECONDS2TICKS(0.01);
		srtSleepUntil(d->t, d->d);
	}	
}


struct data datos,datos2,datos3;
int main(void)
{
	
	
	
	datos.sem=0;
	datos2.sem=0;
	datos3.sem=0;


	//pruebaargs(&datos);
	
/* set LEDs drive state low */
    //LATF  &= 0xFFF0;
    LATD  &= 0xF0FF;

    /* set LEDs pin as output */
    //TRISF &= 0xFFF0;
    TRISD &= 0xF0FF; 
	
	
	// Led naranja
	LATBbits.LATB14 = 0;
	TRISBbits.TRISB14=0;
	LATBbits.LATB14 = 0;
	
	TRISDbits.TRISD1 = 0;
	
	
	
	srtInitKernel(80);
	srtCreateTask(taskreference, 100, SECONDS2TICKS(1), SECONDS2TICKS(0.005), &datos);
	srtCreateTask(taskcontroller, 100, SECONDS2TICKS(0.050), SECONDS2TICKS(0.050), &datos2);
	//srtCreateTask(tasksend, 100, SECONDS2TICKS(0.01), SECONDS2TICKS(0.01), &datos3);
	

	while(1);

	
	CloseTimer1();
	CloseTimer2();
}