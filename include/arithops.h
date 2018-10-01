/*******************************************************************************
*                                                                              *
*      SQL Front-End Interactive System for Gamma & Exodus                     *
*                                                                              *
*                  (c) Dec. 1992 - Manuvir Das, University of Wisconsin        *
*                                                                              *
*      Permission is granted to copy all or part of this code, provided this   *
*      header is present in its entirety in the copied version of the code.    *
*                                                                              *
*      Credits : Ramesh Parameswaran                                           *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*                                                                              *
*      arithops.h : definitions for integer overflow/underflow package         *
*                                                                              *
*******************************************************************************/

#ifndef _ARITHOPS_H_
#define _ARITHOPS_H_

/* Package arithop_package; */
#include <values.h>

/* set for 32 bit reals on Vaxen */
#define MaxInt MAXINT
#define MinInt ((-MAXINT) - 1)
#define MaxReal MAXFLOAT
#define MInReal MINFLOAT

/* oper = (AddOper, SubOper, MulOper, DivOper); */
#define AddOper 0
#define SubOper 1
#define MulOper 2
#define DivOper 3

typedef char oper ;

/* status = (OK, Overflow, ZeroDivide); */
#define OK 0
#define Overflow 1
#define ZeroDivide 2

typedef char status ;


extern void integerop(oper,long int,long int,long int *,status *);

#endif
