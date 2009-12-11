#include "../cab.h"
#include "../simplertk.h"

cab *MyCab;


void taskReader(void *args){
	pointer pread;
	while(1){
		pread=getmes(MyCab);
		unget(MyCab,pread);
		srtSleep(SECONDS2TICKS(0.05), SECONDS2TICKS(0.05));
	}

}
void taskWriter(void *args){
	pointer pw;

	while(1){
		LATBbits.LATB14^= 1;
		pw=reserve(MyCab);
		*pw->data='c';
		*(pw->data+1)='a';
		putmes(MyCab,pw);
		srtSleep(SECONDS2TICKS(0.05), SECONDS2TICKS(0.05));
	}
}


int main(void){
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

	MyCab=opencab(2,2);

	srtInitKernel(180);

	srtCreateTask(taskWriter, 400, SECONDS2TICKS(0.04), SECONDS2TICKS(0.04), 0);
	srtCreateTask(taskReader, 100, SECONDS2TICKS(0.06), SECONDS2TICKS(0.06), 0);
	
	while(1);
	

	
}
