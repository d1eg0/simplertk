#include "cab.h"
#include "simplertk.h"
#include <stdlib.h>



cab *srtOpencab(unsigned int num_buffers, unsigned int dim_buffers){
	cab *c=malloc(sizeof(cab));
	buffer **Buffers=malloc(sizeof(buffer *)*num_buffers);
	
	unsigned int i;
	for(i=0;i<num_buffers;i++){
		Buffers[i]=malloc(sizeof(buffer));
		Buffers[i]->data=malloc(dim_buffers);
		Buffers[i]->use=0;
		if(i<num_buffers-1) Buffers[i]->next=Buffers[i+1];
		else Buffers[i]->next=NIL;
	}
	c->first=Buffers[0];
	c->free=Buffers[0];
	c->mrb=Buffers[num_buffers-1];
	c->max_buf=num_buffers;
	
	return c;	
}

void srtDeletecab(cab *c){
	unsigned int i;
	buffer *Buffers=c->first; //falta coherencia
	for(i=0;i<c->max_buf;i++){
		free(Buffers[i]->data);
		free(Buffers[i]);
	}
	free(c);
}

pointer srtReserve(cab *c){
	pointer p;
	DisableInterrupts();
	p=c->free;
	EnableInterrupts();
	return p;
}

void srtPutmes(cab *c, pointer p){
	DisableInterrupts();
	if (c->mrb->use == 0){
		c->mrb->next = c->free;
		c->free = c->mrb;
	}
	c->mrb=p;
	EnableInterrupts();
}

pointer  srtGetmes(cab *c){
	pointer p;
	DisableInterrupts();
	p=c->mrb;
	p->use++;
	EnableInterrupts();
	return p;
}

void srtUnget(cab *c, pointer p){
	DisableInterrupts();
	p->use--;
	if ((p->use == 0) && (p!= c->mrb)){
		p->next=c->free;
		c->free= p;
	}
	EnableInterrupts();
}
