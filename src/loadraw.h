/*
 * loadraw.h
 *
 *  Created on: Jun 22, 2012
 *      Author: nikolas
 */

#ifndef LOADRAW_H_
#define LOADRAW_H_

typedef unsigned short ushort;

ushort *loadraw(const char *filename, int *w, int *h, float *av, float *tv);


#endif /* LOADRAW_H_ */
