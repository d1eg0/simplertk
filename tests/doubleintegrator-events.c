#include "../simplertk.h"
#include "../uart_dma.h"
#include <math.h>


static float v_max=3.33;  //dsPIC voltage reference
static float Nu =0;
static float Nx[2] ={1.0,0};


static float read_adc1=0;
static float read_adc2=0;

static float reference=0.5;
static float r=0.5;

static float x[2]={0,0};
static float x_hat[2]={0,0};

static float u=0;
static float u_ss=0;


/* Controller gains, u=-k*x */
static float k[2]={1.6158,-0.3360};

// buffer to send information RS232
unsigned char buffer[23];


/*	Variables needed to implement the first order aproximation */
static unsigned int event_time = 10;//next event time

static double a;
static double b=0;//b is always zero in the first order aproximation
static double c;
static double mu=-0.05;
static double M1[2][2]={{1,0},{0,1}};
static double M2[2][2]={{1,0},{0,1}};

static double A[2][2]={{0,-23.8095},{0,0}};
static double B[2][1]={{0},{-23.8095}};
static double L[1][2]={1.6158,-0.3360};//{0.5029,-0.9519};


/* Matrices to implement the first order approximation */
static float xeq[2];
static float xo[2][1];
static float BL[2][2];
static float A_BL[2][2];
static float A_BL_xo[2][1];
static float A_BL_xo_t[1][2];
static float A_BL_xo_t_M1[1][2];
static float xot[1][2];
static float xot_M2[1][2];
static float xot_M2_xo;

void Transpose_Matrix(double *At, double *A, int nrows, int ncols)
{
	double *pA;
	double *pAt;
	int i,j;

	for (i = 0; i < nrows; At += 1, A += ncols, i++) {
		pAt = At;
		pA = A;
		for (j = 0; j < ncols; pAt += nrows, j++) *pAt = pA[j];
	}
}
void Multiply_Matrices(double *C, double *A, int nrows, int ncols, double *B, int mcols)
{
	double *pA = A;
	double *pB;
	double *p_B;
	double *pC = C;
	int i,j,k;

	for (i = 0; i < nrows; A += ncols, i++)
	for (p_B = B, j = 0; j < mcols; C++, p_B++, j++) {
		pB = p_B;
		pA = A;
		*C = 0.0;
		for (k = 0; k < ncols; pA++, pB += mcols, k++)
		*C += *pA * *pB;
	}
}

void Subtract_Matrices(double *C, double *A, double *B, int nrows, int ncols)
{
	register int i;
	register int n = nrows * ncols;

	for (i = 0; i < n; i++) C[i] = A[i] - B[i];
}

float round2nearest(float value)
{
	float max;
	float min;
	max=ceil(value);
	min=floor(value);
	if ((max-value)>(value-min))
	{
		return min;
	}else{
		return max;
	}
}

double Calculate_Next_Activation_Time(void)
{
	double temp_Event_time;
	xeq[0]=reference*Nx[0];
	xeq[1]=0;
	xo[0][0]=x[0]-xeq[0];
	xo[1][0]=x[1]-xeq[1];

	Multiply_Matrices(BL,B,2,1,L,2);
	Subtract_Matrices(A_BL,A,BL,2,2);

	Multiply_Matrices(A_BL_xo,A_BL,2,2,xo,1);

	Transpose_Matrix(A_BL_xo_t, A_BL_xo, 2, 1);

	Multiply_Matrices(A_BL_xo_t_M1,A_BL_xo_t,1,2,M1,2);
	Multiply_Matrices(&a,A_BL_xo_t_M1,1,2,A_BL_xo,1);

	b=0;//In the first order approximation b is always zero

	Transpose_Matrix(xot, xo, 2, 1);
	Multiply_Matrices(xot_M2,xot,1,2,M2,2);
	Multiply_Matrices(&xot_M2_xo,&xot_M2,1,2,xo,1);

	c=mu*xot_M2_xo;

	temp_Event_time=(sqrt(-4.0*a*c))/(2.0*a);
	
	if (temp_Event_time<0.001)//if it is less than 1ms!! configured as the less
	//CounterTick available in this configuration
	{
		temp_Event_time=0.001;
	}

	temp_Event_time=1000*temp_Event_time;


	return temp_Event_time;
}
/* Read capacitors voltages */

void Read_State(void)
{
	while (AD1CON1bits.DONE == 0);	// Waits until ADC1 finish
	read_adc1 = ADC1BUF0;		// Pass register to variable with cast to float?

	while (AD2CON1bits.DONE == 0);	// Waits until ADC2 finish
	read_adc2 = ADC2BUF0;		// Pass register to variable with cast to float?

	x[0]=(read_adc1/(4095))*v_max-v_max/2;//12 bits value to adapted voltage(v_max/2-->0V)
	x[1]=(read_adc2/(4095))*v_max-v_max/2;//12 bits value to adapted voltage(v_max/2-->0V)
}

void taskreference(void *args){

	while(1){

		if (reference == -0.5)
		{
			reference=0.5;
		}else{
			reference=-0.5;
		}
		
		srtSleep(SECONDS2TICKS(1), SECONDS2TICKS(1));
	}	
}

void taskcontroller(void *args){

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

		PDC1= u*0x7FFF/v_max+16600;//Scale to PWM frequency

		// compute next activation
		double next_event=Calculate_Next_Activation_Time();
		event_time=next_event*1000;
		unsigned long next_ticks=SECONDS2TICKS((float)(round2nearest(next_event)/1000.0));
		
		srtSleep( next_ticks,next_ticks);				
	}	
}


void tasksend(void *args){

	static unsigned char *p_r= &r;
	static unsigned char *p_x0= &x[0];
	static unsigned char *p_x1= &x[1];
	static unsigned char *p_u= &u;
	static unsigned char *p_t;
	static unsigned char *p_e=&event_time;

	
	while(1){
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
		buffer[21]=*p_e;
		buffer[22]=*(p_e+1);
		buffer[23]=3;
		buffer[24]=4;

		
		Send(&buffer);

		srtSleep(SECONDS2TICKS(0.01), SECONDS2TICKS(0.01));
	}	
}



int main(void)
{
	PWM_config();
	UART1_DMA_Init();
	ADC1_init();	
	ADC2_init();

	srtInitKernel(100);
	
	srtCreateTask(taskreference, 100, SECONDS2TICKS(1), SECONDS2TICKS(1), 0);
	srtCreateTask(taskcontroller, 100, SECONDS2TICKS(0.010), SECONDS2TICKS(0.010), 0);
	srtCreateTask(tasksend, 800, SECONDS2TICKS(0.01), SECONDS2TICKS(0.01), 0);
	

	while(1);

}