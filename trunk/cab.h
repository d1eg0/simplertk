/*! \file cab.h
    \brief This file includes the Simple Real Time Kernel CAB communication mechanism.
*/

#ifndef _CAB_
#define _CAB_

#define NIL 0

typedef struct buffer buffer;
typedef struct cab cab;
typedef struct buffer *pointer;

/*!
\struct buffer
\brief Buffer of data
*/
struct buffer{
	pointer next; /**< pointer to next buffer */
	unsigned int use; /**< number of readers reading this buffer */
	char *data; /**< pointer to the data */
};

/*!
\struct cab
\brief CAB Structure (Cyclical Asinchronous Buffers)
*/
struct cab{
	/*! pointers to buffers */
	buffer **buffers;
	/*! first buffer */
	pointer first;
	/*! free buffer */
	pointer free; 
	/*! most recent buffer */
	pointer mrb; 
	/*! maximum buffers */
	unsigned int max_buf; 
	/*! buffers dimension */
	unsigned int dim_buf; 
};

/*! 
\brief Creates a CAB structure. 
\param num_buffers number of buffers
\param dim_buffers dimension of each data buffer
\return Pointer to the new cab
*/
cab *srtOpencab(unsigned int num_buffers, unsigned int dim_buffers);

/*! 
\brief Delete a  CAB 
\param c CAB pointer to be deleted
*/
void srtDeletecab(cab *c);

/*! 
\brief Reserve a free buffer to write data
\param c CAB pointer to reserve the buffer
\return Pointer to the buffer reserved
*/
pointer srtReserve(cab *c);

/*! 
\brief Update the most recent buffer
\param c CAB pointer destiny
\param p Pointer to the new most recent buffer
*/
void srtPutmes(cab *c, pointer p);

/*! 
\brief Reserve the most recent buffer to read data
\param c CAB pointer
\return Pointer to the most recent buffer
*/
pointer srtGetmes(cab *c);

/*! 
\brief Release the readed buffer
\param c CAB pointer
\param p Pointer to the buffer to be released
*/
void srtUnget(cab *c, pointer p);

#endif

