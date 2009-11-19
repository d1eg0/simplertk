#include "simplertk.h"

_FOSCSEL(FNOSC_PRIPLL);// Primary (XT, HS, EC) Oscillator with PLL
_FOSC(OSCIOFNC_ON & POSCMD_XT);// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystanl
_FWDT(FWDTEN_OFF);// Watchdog Timer Enabled/disabled by user software
_FGS(GCP_OFF);// Disable Code Protection

/************** KERNEL DATA STRUCTURES *******************/
struct task {
  unsigned int sp;		// Stack pointer
  unsigned long release;
  unsigned long deadline;
  unsigned char state;     // 0=terminated, 1=readyQ, 2=timeQ
                     // 3=waiting for Sem1, 4=waiting for Sem2, etc.
};

struct kernel {
  unsigned char nbrOfTasks; // number of tasks created so far
  unsigned char running;
  struct task tasks[MAXNBRTASKS+1]; // +1 for the idle task
  unsigned char semaphores[MAXNBRSEMAPHORES]; // counters for semaphores
  unsigned int *memptr; // pointer to free memory
  volatile unsigned long cycles;  // number of major cycles since system start

  unsigned long nextHit; // next kernel wake-up time
} kernel;

static int current_cpu_ipl;

void __attribute__((__interrupt__,__auto_psv__)) _T2Interrupt(void){
		int a=0;
}

void EnableInterrupts(){
	RESTORE_CPU_IPL(current_cpu_ipl); 
	__asm__ volatile("disi #0x0000"); /* enable interrupts */ 
}

void DisableInterrupts(){
	SET_AND_SAVE_CPU_IPL(current_cpu_ipl, 7);  /* disable level 7 interrupts */ 
	__asm__ volatile("disi #0x3FFF"); /* disable interrupts */ 
}

volatile unsigned int sptemp;

 char scheduler(){
	unsigned char running, oldrunning;
	struct task *t;
	unsigned char i;
	unsigned long now;
	unsigned long nextHit;
	long timeleft;
	

	//add cycle if timer value
	if ((IFS0bits.T1IF == 1) && (PR1==TIMER_VALUE)){
		kernel.cycles++;
	}
	
	nextHit = 0x7FFFFFFF;
	oldrunning = kernel.running;
	running = 0;
 
	//Read clock
	now = (kernel.cycles * TIMER_VALUE) + ReadTimer1();	
	
	//Release tasks from TimeQ and determine new running task

	for (i=1; i <= kernel.nbrOfTasks; i++) {
		t = &kernel.tasks[i];
		if (t->state == TIMEQ) {
			if (t->release <= now) {
				t->state = READYQ;
			} else if (t->release < nextHit) {
				nextHit = t->release;
			}
		}
		if (t->state == READYQ) {
			if (t->deadline < kernel.tasks[running].deadline) {
				running = i;
			}
		}
	}
	
	if (running != oldrunning) { // perform context switch?
		//store old context
		t = &kernel.tasks[oldrunning];
		t->sp=sptemp;
	
		kernel.running = running;
		
	}else{
		running=NO_CONTEXT_SWITCH; // no context switch
	}
	
	kernel.nextHit = nextHit;  

	now = (kernel.cycles * TIMER_VALUE) + ReadTimer1();
	timeleft = nextHit - now;
	if (timeleft < 4) {
		timeleft = 4;
	}

	if ((unsigned long)ReadTimer1() + timeleft <(TIMER_VALUE+1)) {
		PR1 = ReadTimer1() + timeleft;
	} else if (ReadTimer1() < (TIMER_VALUE+1) - 4) {
		PR1 = TIMER_VALUE;
	} else {
		PR1 = 4;
	}
	
	return running;
}

void dispatch(void){
	struct task *t;
	t = &kernel.tasks[kernel.running];
	sptemp=t->sp;
}


void srtInitKernel(int idlestack){
	// Clock setup for 40MIPS 
	CLKDIVbits.DOZEN   = 0;
	CLKDIVbits.PLLPRE  = 0;
	CLKDIVbits.PLLPOST = 0;
	PLLFBDbits.PLLDIV  = 78;
	
	/* Wait for PLL to lock */
	while(OSCCONbits.LOCK!=1);
	
	kernel.memptr= (void*)(SPLIM - idlestack );
	kernel.nbrOfTasks= 0;
	kernel.running = 0;
	kernel.cycles = 0x00000000;

	kernel.nextHit = 0x7FFFFFFF;
	
	kernel.tasks[0].sp=kernel.memptr;
	kernel.tasks[0].deadline = 0x7FFFFFFF;
	kernel.tasks[0].release = 0x00000000;
	
	
	/* Enable Timer1 Interrupt and Priority to "7" */
	ConfigIntTimer1(T1_INT_PRIOR_7 & T1_INT_ON);
	WriteTimer1(0);

	unsigned int timer_prescaler1;
	switch (PRESCALER){
		case 1:
			timer_prescaler1=T1_PS_1_1;
			break;
		case 8:
			timer_prescaler1=T1_PS_1_8;
			break;
		case 64:
			timer_prescaler1=T1_PS_1_64;
			break;
		case 256:
			timer_prescaler1=T1_PS_1_256;
	}
	OpenTimer1(T1_ON & T1_GATE_OFF & T1_IDLE_STOP &
		timer_prescaler1 & T1_SYNC_EXT_OFF &
		T1_SOURCE_INT, TIMER_VALUE);
	

	
	/* set pin (AN10/RB10)-->(CON6/Pin28) drive state low */
	LATBbits.LATB10 = 0;
	/* set pin (AN10/RB10)-->(CON6/Pin28) as output */
	TRISBbits.TRISB10 = 0;

}
	
extern void _T1Interrupt(void);

void srtCreateTask(void (*fun)(void*), unsigned int stacksize, unsigned long release, unsigned long deadline, void* args){
	unsigned int *sp;
	struct task *t;
	int i;

	DisableInterrupts();// turn off interrupts

	++kernel.nbrOfTasks;

	
	kernel.memptr -= stacksize;  // decrease free mem ptr
	sp = kernel.memptr;
	
	
	
	
	*sp++ = fun;    // store PC
	*sp++ = 0x0000; // SR<7:0> IPL3,CORCON<3> PC<22:16>
	
	*sp++=SPLIM; //SPLIM
	*sp++=0x0000; //SR

	*sp++ = kernel.memptr+24; 	// w14
	// initialize stack
	for(i=0;i<12;i++)
		*sp++ = 0x0000; // w13-w2

	*sp++ = args; 	// wo
	*sp++ = 0x0000; // w1

	
	*sp++=0x0000; //RCOUNT
	*sp++=0x0000; //TBLPAG
	*sp++=0x0000; //CORCON
	*sp++=0x0000; //PSVPAG


	
	


	t = &kernel.tasks[kernel.nbrOfTasks];
	t->sp=sp;	// store stack pointer
	t->release = release;
	t->deadline = deadline;
	t->state = TIMEQ;
  
	EnableInterrupts();
	_T1Interrupt(); // call interrupt handler to schedule

}


void srtCreateSemaphore(unsigned char semnbr, unsigned char initVal) {

  DisableInterrupts(); // turn off interrupts

  kernel.semaphores[semnbr-1] = initVal;
  
  EnableInterrupts(); // set enabled interrupts;
}


void srtWait(unsigned char semnbr) {

	struct task *t;
	unsigned char *s;

	t = &kernel.tasks[kernel.running];

	DisableInterrupts(); // disable interrupts

	s = &kernel.semaphores[semnbr-1];
	if ((*s) > 0) {
	(*s)--;
	} else {

	t->state = semnbr + WAIT_OFFSET; // waiting for Sem#semnbr
	_T1Interrupt(); // call interrupt handler to schedule
	}

	EnableInterrupts(); // reenable interrupts
}

void srtSignal(unsigned char semnbr) {

  unsigned char i;
  struct task *t;
  unsigned long minDeadline = 0xFFFFFFFF;
  unsigned char taskToReadyQ = 0;

  DisableInterrupts(); // disable interrupts

  for (i=1; i <= kernel.nbrOfTasks; i++) {
    t = &kernel.tasks[i];
    if (t->state == (semnbr + WAIT_OFFSET)) {
      if (t->deadline <= minDeadline) {
	taskToReadyQ = i;
	minDeadline = t->deadline;
      }
    }
  }

  if (taskToReadyQ == 0) {
    kernel.semaphores[semnbr-1]++;
  } else {
    kernel.tasks[taskToReadyQ].state = READYQ; // make task ready
    _T1Interrupt(); // call interrupt handler to schedule
  }

  EnableInterrupts(); // reenable interrupts
}

unsigned long srtCurrentTime(void) {
	return (kernel.cycles * TIMER_VALUE) + ReadTimer1();
}

static void restartCycle(void){
	unsigned char i;
	unsigned long min_time=0xFFFFFFFF;
	unsigned char task_ref=0;
	DisableInterrupts(); // turn off interrupts
	
	for (i=1; i <= kernel.nbrOfTasks; i++) {
		if((kernel.tasks[i].state!=TERMINATED) & (kernel.tasks[i].release<min_time) ){
			task_ref=i;
			min_time=kernel.tasks[i].release;
		}
	}
	
	kernel.tasks[task_ref].release=0;
	kernel.tasks[task_ref].deadline-=min_time;
	
	for (i=1; i <= kernel.nbrOfTasks; i++) {
		if ((kernel.tasks[i].state!=TERMINATED) & (i!=task_ref)){
			kernel.tasks[i].release-=min_time;
			kernel.tasks[i].deadline-=min_time;
		}
	}
	kernel.cycles=0;
	EnableInterrupts();
}

void srtSleep(unsigned long release, unsigned long deadline) {

	struct task *t;
	unsigned long temp_res;
	t = &kernel.tasks[kernel.running];

	DisableInterrupts(); // turn off interrupts

	t->state = TIMEQ;
	
	temp_res=t->deadline+deadline; //worst overflow case test
	if(SR&0x0004)restartCycle(); //status register overflow bit -> restart life counters
	t->deadline+= deadline;
	t->release+= release;

	EnableInterrupts();
	_T1Interrupt();	// call interrupt handler to schedule
}


unsigned long srtGetRelease(void) {
	return kernel.tasks[kernel.running].release;
}

unsigned long srtGetDeadline(void) {
	return kernel.tasks[kernel.running].deadline;
}

void srtTerminate(void) {

	DisableInterrupts();

	kernel.tasks[kernel.running].state = TERMINATED;
	EnableInterrupts();
	_T1Interrupt(); // call interrupt handler to schedule
}



