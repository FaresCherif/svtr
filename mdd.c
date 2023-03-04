/*
 * mdd.c
 *
 *  Created on: 25 janv. 2017
 *      Author: grolleau
 */
#include <pthread.h>
#include <string.h>
#include <malloc.h>
#include "mdd.h"

/**
 * Initializes a shared int
 *\param val[in] initial value of the shared data
 *\return the new shared int, memory allocation is done inside the function
 */
MDD_int MDD_int_init(const int val) {
	MDD_int m=(MDD_int)malloc(sizeof(struct s_MDD_int));
	m->val=val;
	m->dirty=0;
	pthread_mutex_init(&(m->mutex),0);
	return m;
}

/**
 * Writes in a shared int
 *\param mdd[in,out] the shared data
 *\param val[in] value to write in the shared data
 */
void MDD_int_write(MDD_int mdd, const int val) {
	pthread_mutex_lock(&(mdd->mutex));
	mdd->val=val;
	mdd->dirty=1;
	pthread_mutex_unlock(&(mdd->mutex));
}
/**
 * Reads in a shared int
 *\param mdd[in] the shared data
 *\return value of the shared data
 */
int MDD_int_read(MDD_int mdd) {
	int val;
	pthread_mutex_lock(&(mdd->mutex));
	val=mdd->val;
	mdd->dirty=0;
	pthread_mutex_unlock(&(mdd->mutex));
	return val;
}
/**
 * Reads in a shared int, while returning if it was dirty
 *\param mdd[in] the shared data
 *\param val[out] value of the shared data
 *\return  1 if the shared data was modified since last reading (i.e., dirty), 0 else
 */
int MDD_int_read2(MDD_int mdd,int *val) {
	int dirty;
	pthread_mutex_lock(&(mdd->mutex));
	*val=mdd->val;
	dirty=mdd->dirty;
	mdd->dirty=0;
	pthread_mutex_unlock(&(mdd->mutex));
	return dirty;

}







MDD_pos MDD_pos_init(const int x,const int y,const int ang) {
	MDD_pos m=(MDD_pos)malloc(sizeof(struct s_MDD_pos));
	m->x=x;
	m->x=y;
	m->x=ang;
	m->dirty=0;
	pthread_mutex_init(&(m->mutex),0);
	return m;
}

void MDD_pos_write(MDD_pos mdd, const int x,const int y,const int ang){
	pthread_mutex_lock(&(mdd->mutex));
	mdd->x=x;
	mdd->x=y;
	mdd->x=ang;
	mdd->dirty=1;
	pthread_mutex_unlock(&(mdd->mutex));
}

int* MDD_pos_read(MDD_pos mdd){
	int* pos;

	pthread_mutex_lock(&(mdd->mutex));
	pos[0]=mdd->x;
	pos[1]=mdd->y;
	pos[2]=mdd->ang;
	mdd->dirty=0;
	pthread_mutex_unlock(&(mdd->mutex));
	return pos;
}

int MDD_pos_read2(MDD_pos mdd,int *x,int *y,int *ang) {
	int dirty;
	pthread_mutex_lock(&(mdd->mutex));
	*x=mdd->x;
	*y=mdd->y;
	*ang=mdd->ang;

	dirty=mdd->dirty;
	mdd->dirty=0;
	pthread_mutex_unlock(&(mdd->mutex));
	return dirty;

}