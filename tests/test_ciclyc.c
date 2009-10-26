#include "../simplertk.h"






static float reference=2;
static float r=2;



struct data{
	unsigned int sem;
	unsigned long d;
	unsigned long t;
};
struct data* d;
struct data* d2;
struct data* d3;
struct data* d4;



void taskreference(void *args){
	d2=(struct data*)args;
	
	d2->t = srtGetRelease();
	d2->d = srtGetDeadline();
	while(1){
		LATBbits.LATB10 ^=1;

		if (reference == 1.0)
		{
			reference=2.0;
			LATBbits.LATB14 = 1;//Orange led switch on
		}else{
			reference=1.0;
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
		srtSleepUntil(SECONDS2TICKS(0.05), SECONDS2TICKS(0.05));
	}	
}



/*void tasksend(void *args){

	static unsigned char *p_r= &r;
	static unsigned char *p_x0= &x[0];
	static unsigned char *p_x1= &x[1];
	static unsigned char *p_u= &u;
	static unsigned char *p_t;
	//LATBbits.LATB10 = 1; //To get time with the oscilloscope

	//Read_State();
	//static unsigned char *ptr,*ptrn,*ptrd;
	d=(struct data*)args;
	
	d->t = trtGetRelease();
	d->d = trtGetDeadline();
	while(1){
						

		//LATBbits.LATB10 ^=1;

		Read_State();
		unsigned long now=trtGetNow();
		p_t=&now;
		p_r=&r;
		p_x0=&x[0];
		p_x1=&x[1];
		p_u=&u;
		
		BufferOutput[0]=*p_r;
		BufferOutput[1]=*(p_r+1);
		BufferOutput[2]=*(p_r+2);
		BufferOutput[3]=*(p_r+3);
		BufferOutput[4]=*p_x0;
		BufferOutput[5]=*(p_x0+1);
		BufferOutput[6]=*(p_x0+2);
		BufferOutput[7]=*(p_x0+3);
		BufferOutput[8]=*p_x1;
		BufferOutput[9]=*(p_x1+1);
		BufferOutput[10]=*(p_x1+2);
		BufferOutput[11]=*(p_x1+3);
		BufferOutput[12]=*p_u;
		BufferOutput[13]=*(p_u+1);
		BufferOutput[14]=*(p_u+2);
		BufferOutput[15]=*(p_u+3);
		BufferOutput[16]=*p_t;
		BufferOutput[17]=*(p_t+1);
		BufferOutput[18]=*(p_t+2);
		BufferOutput[19]=*(p_t+3);
		BufferOutput[20]=1;
		BufferOutput[21]=2;
		BufferOutput[22]=3;
		BufferOutput[23]=4;

		
		//Send(&buffer);
		 //Force sending data
		DMA4CONbits.CHEN  = 1;            // Re-enable DMA4 Channel
		DMA4REQbits.FORCE = 1;            // Manual mode: Kick-start the first transfer

		
		//LATBbits.LATB14 = 1;
		d->t+= SECONDS2TICKS(0.01);
		d->d+= SECONDS2TICKS(0.01);
		trtSleepUntil(d->t, d->d);
	}	
}*/


struct data datos,datos2,datos3;
int main(void)
{

	
	
	datos.sem=0;
	datos2.sem=0;
	datos3.sem=0;


	//pruebaargs(&datos);
	
/* set LEDs drive state low */
    LATF  &= 0xFFF0;
    LATD  &= 0xF0FF;

    /* set LEDs pin as output */
    TRISF &= 0xFFF0;
    TRISD &= 0xF0FF; 
	
	
	// Led naranja
	LATBbits.LATB14 = 0;
	TRISBbits.TRISB14=0;
	LATBbits.LATB14 = 0;
	
	TRISDbits.TRISD1 = 0;
	
	
	
	srtInitKernel(80);
	srtCreateTask(taskreference, 100, SECONDS2TICKS(0.05), SECONDS2TICKS(0.05), &datos);
	//trtCreateTask(taskcontroller, 100, SECONDS2TICKS(0.050), SECONDS2TICKS(0.050), &datos2);
	//trtCreateTask(tasksend, 100, SECONDS2TICKS(0.1), SECONDS2TICKS(0.01), &datos3);
	// trtCreateTask(prueba4, 100, SECONDS2TICKS(0.03), SECONDS2TICKS(0.05), &datos4);
// trtCreateTask(prueba5, 100, SECONDS2TICKS(0.02), SECONDS2TICKS(0.04), &datos5);
// trtCreateTask(prueba6, 100, SECONDS2TICKS(0.01), SECONDS2TICKS(0.03), &datos6);


	while(1);
	
	CloseTimer1();
}