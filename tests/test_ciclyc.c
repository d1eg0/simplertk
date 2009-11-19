#include "../simplertk.h"






static float reference=2;
static float r=2;



struct data{
	unsigned int sem;
	unsigned long d;
	unsigned long t;
	unsigned int r;
};
struct data* d;
struct data* d2;
struct data* d3;
struct data* d4;
struct data* d5;


void taskprueba2(void *args){
	d2=(struct data*)args;
	
	d2->t = srtGetRelease();
	d2->d = srtGetDeadline();
	while(1){
		//LATBbits.LATB10 ^=1;

		if (d2->r == 1)
		{
			d2->r=2;
			//LATBbits.LATB14 = 1;//Orange led switch on
		}else{
			d2->r=1;
			//LATBbits.LATB14 = 0;//Orange led switch off
		}
		
		srtSleep(SECONDS2TICKS(0.06), SECONDS2TICKS(0.06));
	}
}

void taskprueba3(void *args){
	d3=(struct data*)args;
	
	d3->t = srtGetRelease();
	d3->d = srtGetDeadline();
	while(1){
		//LATBbits.LATB10 ^=1;

		if (d3->r == 1)
		{
			d3->r=2;
			//LATBbits.LATB14 = 1;//Orange led switch on
		}else{
			d3->r=1;
			//LATBbits.LATB14 = 0;//Orange led switch off
		}
		
		srtSleep(SECONDS2TICKS(0.09), SECONDS2TICKS(0.09));
	}
}

void taskprueba4(void *args){
	d4=(struct data*)args;
	
	d4->t = srtGetRelease();
	d4->d = srtGetDeadline();
	while(1){
		//LATBbits.LATB10 ^=1;

		if (d4->r == 1)
		{
			d4->r=2;
			//LATBbits.LATB14 = 1;//Orange led switch on
		}else{
			d4->r=1;
			//LATBbits.LATB14 = 0;//Orange led switch off
		}
		
		srtSleep(SECONDS2TICKS(0.07), SECONDS2TICKS(0.07));
	}
}

void taskprueba5(void *args){
	d5=(struct data*)args;
	
	d5->t = srtGetRelease();
	d5->d = srtGetDeadline();
	while(1){
		LATBbits.LATB10 ^=1;

		if (d5->r == 1)
		{
			d5->r=2;
			LATBbits.LATB14 = 1;//Orange led switch on
		}else{
			d5->r=1;
			LATBbits.LATB14 = 0;//Orange led switch off
		}
		
		srtSleep(SECONDS2TICKS(0.08), SECONDS2TICKS(0.08));
	}
}

void taskreference(void *args){
	d=(struct data*)args;
	
	d->t = srtGetRelease();
	d->d = srtGetDeadline();
	while(1){
		//LATBbits.LATB10 ^=1;

		if (reference == 1.0)
		{
			reference=2.0;
			//LATBbits.LATB14 = 1;//Orange led switch on
		}else{
			reference=1.0;
			//LATBbits.LATB14 = 0;//Orange led switch off
		}

		srtSleep(SECONDS2TICKS(0.05), SECONDS2TICKS(0.05));
	}	
}

struct data datos3,datos2, datos4,datos5,datos6;
int main(void)
{

	
	
	datos2.sem=0;
	datos2.r=2;
	datos3.sem=0;
	datos3.r=2;
	datos4.sem=0;
	datos4.r=2;
	datos5.sem=0;
	datos5.r=2;
	datos6.sem=0;
	datos6.r=2;
	

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
	
	
	
	srtInitKernel(180);
	
	srtCreateTask(taskprueba2, 100, SECONDS2TICKS(0.06), SECONDS2TICKS(0.06), &datos2);
	srtCreateTask(taskprueba3, 100, SECONDS2TICKS(0.09), SECONDS2TICKS(0.09), &datos3);
	srtCreateTask(taskprueba4, 100, SECONDS2TICKS(0.07), SECONDS2TICKS(0.07), &datos4);
	srtCreateTask(taskprueba5, 100, SECONDS2TICKS(0.08), SECONDS2TICKS(0.08), &datos5);

	srtCreateTask(taskreference, 100, SECONDS2TICKS(0.05), SECONDS2TICKS(0.05), &datos6);
	//trtCreateTask(taskcontroller, 100, SECONDS2TICKS(0.050), SECONDS2TICKS(0.050), &datos2);
	//trtCreateTask(tasksend, 100, SECONDS2TICKS(0.1), SECONDS2TICKS(0.01), &datos3);
	// trtCreateTask(prueba4, 100, SECONDS2TICKS(0.03), SECONDS2TICKS(0.05), &datos4);
// trtCreateTask(prueba5, 100, SECONDS2TICKS(0.02), SECONDS2TICKS(0.04), &datos5);
// trtCreateTask(prueba6, 100, SECONDS2TICKS(0.01), SECONDS2TICKS(0.03), &datos6);


	while(1);
	
	CloseTimer1();
}