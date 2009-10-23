/*! \file simplertk.h
    \brief This file includes the Simple Real Time Kernel functions declarations.
*/

#ifndef _SIMPLERTK_
#define _SIMPLERTK_

#include <p33fj256mc710.h>
#include <timer.h>
#include "setup.h"
#include "uart_dma.h"




#define MAXNBRTASKS 6
#define MAXNBRSEMAPHORES 6

#define PRESCALER 8
/*
PRESCALER	PERIOD		TIMER VALUE
	1 		1ms 	-> 	0x9C40
	8 		10ms 	-> 	0xC350
	256 	100ms 	-> 	0x3D09
	256 	10ms 	-> 	0x061A
*/
#define TIMER_VALUE 0x9C40
#define TICKSPERSECOND (40E6 / PRESCALER)

#define TERMINATED 0
#define READYQ 1
#define TIMEQ 2
#define WAIT_OFFSET 2

/*!
\brief Change seconds to ticks
\param T seconds
*/
#define SECONDS2TICKS(T) ((unsigned long)((T)*TICKSPERSECOND))


#define RAMEND 0x7FFF

/************** KERNEL DATA STRUCTURES *******************/
struct task;

struct kernel;

// Disable all interrupts 
void DisableInterrupts();
// Enable all interrupts
void EnableInterrupts();

/**************** CLOCK INTERRUPT HANDLER *****************/

void __attribute__((__interrupt__)) _T1Interrupt(void);


void __attribute__((__interrupt__)) _T2Interrupt(void);

/***************** API *****************/
/*! 
\brief Initialize the kernel 
\param idlestack size of the idle stack
*/
void srtInitKernel(int idlestack);
	

/*! 
\brief Create a task
\param fun task source code adress
\param stacksize task stack size
\param release task release (ticks)
\param deadline task deadline (ticks)
\param args task arguments adress
*/
void srtCreateTask(void (*fun)(void*), unsigned int stacksize, unsigned long release, unsigned long deadline, void* args);

/*! 
\brief Create a semaphore
\param semnbr semaphore identifier
\param initVal initial value
*/
void srtCreateSemaphore(unsigned char semnbr, unsigned char initVal);

/*! 
\brief Wait on a semaphore
\param semnbr semaphore identifier
*/
void srtWait(unsigned char semnbr);


/*! 
\brief Signal a semaphore
\param semnbr semaphore identifier
*/
void srtSignal(unsigned char semnbr);

/** Get the current system time (ticks) */
unsigned long srtCurrentTime(void);

/*! 
\brief Put a task to sleep until a certain time
\param release the release time of the task
\param deadline the deadline time of the task
*/
void srtSleepUntil(unsigned long release, unsigned long deadline);

/** Get the release time (ticks) of the running task */
unsigned long srtGetRelease(void);

/** Get the deadline (ticks) of the running task */
unsigned long srtGetDeadline(void);

/** Terminate the execution of the current task */
void srtTerminate(void);


#endif
