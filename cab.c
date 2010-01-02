#include "cab.h"
#include "simplertk.h"
#include <stdlib.h>




cab *srtOpencab(unsigned int num_buffers, unsigned int dim_buffers){
	cab *c=malloc(sizeof(cab));
	c->buffers=malloc(sizeof(buffer *)*num_buffers);
	
	unsigned int i;
	for(i=0;i<num_buffers;i++){
		c->buffers[i]=malloc(sizeof(buffer));
		c->buffers[i]->data=malloc(dim_buffers);
		c->buffers[i]->use=0;
		if(i<num_buffers-1) c->buffers[i]->next=c->buffers[i+1];
		else c->buffers[i]->next=NIL;
	}
	c->first=c->buffers[0];
	c->free=c->buffers[0];
	c->mrb=c->buffers[num_buffers-1];
	c->max_buf=num_buffers;
	
	return c;	
}

void srtDeletecab(cab *c){
	unsigned int i;
	// release memory of the cab
	for(i=0;i<c->max_buf;i++){
		free(c->buffers[i]->data);
		free(c->buffers[i]);
	}
	free(c);
}

pointer srtReserve(cab *c){
	pointer p;
	DisableInterrupts();
	p=c->free; //returns the free buffer
	EnableInterrupts();
	return p;
}

void srtPutmes(cab *c, pointer p){
	DisableInterrupts();
	//update the free buffer
	if (c->mrb->use == 0){
		c->mrb->next = c->free;
		c->free = c->mrb;
	}
	//update de mrb buffer
	c->mrb=p;
	EnableInterrupts();
}

pointer  srtGetmes(cab *c){
	pointer p;
	DisableInterrupts();
	//return the mrb buffer for use it
	p=c->mrb;
	p->use++;
	EnableInterrupts();
	return p;
}

void srtUnget(cab *c, pointer p){
	DisableInterrupts();
	//release the buffer
	p->use--;
	//if it is the last using the mrb buffer and
	// there is new one -> updates the free buffer
	if ((p->use == 0) && (p!= c->mrb)){
		p->next=c->free;
		c->free= p;
	}
	EnableInterrupts();
}
