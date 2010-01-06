#include "../simplertk.h"
#include "../uart_dma.h"

unsigned char buffer[24];

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
		delay(990);
		srtSleep(SECONDS2TICKS(0.01), SECONDS2TICKS(0.01));
		
	}
}

void taskLoad2(void *args){
	unsigned char *p_t;
	
	while(1){
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


int main(void)
{
	//Communication RS232
	UART1_DMA_Init();

	
	srtInitKernel(80);

	srtCreateTask(taskLoad1, 100, SECONDS2TICKS(0.011), SECONDS2TICKS(0.02), 0);
	srtCreateTask(taskLoad1, 100, SECONDS2TICKS(0.012), SECONDS2TICKS(0.02), 0);
	srtCreateTask(taskLoad1, 100, SECONDS2TICKS(0.013), SECONDS2TICKS(0.02), 0);
	srtCreateTask(taskLoad1, 100, SECONDS2TICKS(0.014), SECONDS2TICKS(0.02), 0);
	srtCreateTask(taskLoad1, 100, SECONDS2TICKS(0.015), SECONDS2TICKS(0.02), 0);
	srtCreateTask(taskLoad1, 100, SECONDS2TICKS(0.016), SECONDS2TICKS(0.02), 0);
	srtCreateTask(taskLoad1, 100, SECONDS2TICKS(0.017), SECONDS2TICKS(0.02), 0);
	srtCreateTask(taskLoad1, 100, SECONDS2TICKS(0.018), SECONDS2TICKS(0.02), 0);
	srtCreateTask(taskLoad1, 100, SECONDS2TICKS(0.019), SECONDS2TICKS(0.02), 0);

	srtCreateTask(taskLoad2, 100, SECONDS2TICKS(0.01), SECONDS2TICKS(0.02), 0);

	for (;;);
	
	return 0;
}
