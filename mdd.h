/*
 * mdd.h
 *
 *  Created on: 25 janv. 2017
 *      Author: grolleau
 */

#ifndef MDD_H_
#define MDD_H_

typedef struct s_MDD_int {
	int val;
	int dirty;
	pthread_mutex_t mutex;
} * MDD_int;

/**
 * Initializes a shared int
 *\param val[in] initial value of the shared data
 *\return the new shared int, memory allocation is done inside the function
 */
MDD_int MDD_int_init(const int val);

/**
 * Writes in a shared int
 *\param mdd[in,out] the shared data
 *\param val[in] value to write in the shared data
 * Side effect: shared int is considered dirty until next reading
 */
void MDD_int_write(MDD_int mdd, const int val);

/**
 * Reads in a shared int
 *\param mdd[in] the shared data
 *\return value of the shared data
 * Side effect: shared int is now considered clean
 */
int MDD_int_read(MDD_int mdd);

/**
 * Reads in a shared int, while returning if it was dirty
 *\param mdd[in] the shared data
 *\param val[out] value of the shared data
 *\return  1 if the shared data was modified since last reading (i.e., dirty), 0 else
 * Side effect: shared int is now considered clean
 */
int MDD_int_read2(MDD_int mdd,int *val);









typedef struct s_MDD_pos {
	int x;
	int y;
	int ang;
	int dirty;
	pthread_mutex_t mutex;
} * MDD_pos;

/**
 * Initializes a shared pos
 *\param x[in] initial value x of the shared data
 *\param y[in] initial value y of the shared data
 *\param ang[in] initial value ang of the shared data
 *\return the new shared pos, memory allocation is done inside the function
 */
MDD_pos MDD_pos_init(const int x,const int y,const int ang);

/**
 * Writes in a shared int
 *\param mdd[in,out] the shared data
 *\param x[in] x to write in the shared data
 *\param y[in] y to write in the shared data
 *\param ang[in] ang to write in the shared data
 * Side effect: shared pos is considered dirty until next reading
 */
void MDD_pos_write(MDD_pos mdd, const int x,const int y,const int ang);

/**
 * Reads in a shared int
 *\param mdd[in] the shared data
 *\return value of the shared data
 * Side effect: shared pos is now considered clean
 */
int* MDD_pos_read(MDD_pos mdd);

/**
 * Reads in a shared int, while returning if it was dirty
 *\param mdd[in] the shared data
 *\param x[out] x of the shared data
 *\param y[out] y of the shared data
 *\param ang[out] ang of the shared data
 *\return  1 if the shared data was modified since last reading (i.e., dirty), 0 else
 * Side effect: shared pos is now considered clean
 */
int MDD_pos_read2(MDD_pos mdd,int *x,int *y,int *ang);

#endif /* MDD_H_ */
