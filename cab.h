#ifndef _CAB_
#define _CAB_

#define NIL 0

typedef struct buffer buffer;
typedef struct cab cab;
typedef struct buffer *pointer;

struct buffer{
	pointer next;
	unsigned int use;
	char *data;
};


struct cab{
	pointer free; //free buffer
	pointer mrb; // most recent buffer
	unsigned int max_buf; //maximum buffers
	unsigned int dim_buf; //buffers dimension
};

cab *opencab(unsigned int num_buffers, unsigned int dim_buffers);

void deletecab(cab *c);

pointer reserve(cab *c);

void putmes(cab *c, pointer p);

pointer getmes(cab *c);

void unget(cab *c, pointer p);

#endif

