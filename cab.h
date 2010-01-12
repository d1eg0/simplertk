/*
	SimpleRTK: a real time kernel for dspic
    Copyright (C) 2009  Diego García

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
    
    mail: kobydiego@gmail.com
*/
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
	buffer **buffers; /**< pointers to buffers */
	pointer free; /**< free buffer */
	pointer mrb; /**< most recent buffer */
	unsigned int max_buf; /**< maximum buffers */
	unsigned int dim_buf; /**< buffers dimension */
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

