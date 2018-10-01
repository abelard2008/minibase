
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
*      arithops.c : integer routines for the lexical analyzer                  *
*                                                                              *
*******************************************************************************/

#include <math.h>
#include <arithops.h>
#include <stdlib.h>

long labsolute( long x)
{
    if (x >= 0)
	return x;
    else
	return -x;
}


void integerop(oper Op, long int A, long int B, long int *C, status *Flag)
{
	/* for div */
   float x,y,z;
   switch (Op) {
	
	case AddOper: {
			if ((A == 0) || (B == 0)) {
				*Flag = OK;
				*C = A+B;
			}
			else if ((A > 0) && (B > 0)) {
				if ((MaxInt -A) >= B)  {
					*Flag = OK;
					*C = A+B;
				}
				else
					*Flag = Overflow;
			     }
			else if ((A < 0) && (B < 0))  {
				if (( MinInt + -A) <= B)  {
					*Flag = OK;
					*C = A+B;
				}
				else
					*Flag = Overflow;
			     }
			else {
				*Flag = OK;
				*C = A+B;
			}
			break;
		}
	   

	case SubOper: {
			if ((A >= 0) && (B < 0))  {
				if ((MaxInt + B) >= A)  {
					*Flag = OK;
					*C = A-B;
				}
				else	*Flag = Overflow;
			}
			else if ((A < 0) && (B >= 0))  {
				if (( (long) MinInt + B)  <= A)  {
					*Flag = OK;
					*C = A-B;
				}
				else
					*Flag = Overflow;
			     }
			else  {
				*Flag = OK;
				*C = A-B;
			}
			break;
		}
	   

	case MulOper: {
			if ((A == 0) || (B == 0))  {
				*Flag = OK;
				*C = 0;
			}
			else if (A == -1)
				integerop(SubOper, 0, B, C, Flag);
			else if (B == -1)
				integerop(SubOper, 0, A, C, Flag);
			else if (MaxInt / labsolute(A) < labsolute(B))
				*Flag = Overflow;
			else {
				*Flag = OK;
				*C = A*B;
			}
			break;
		}

	case DivOper: {
			if (B == 0)
				*Flag = ZeroDivide;
			else if (B == -1) 
				integerop(SubOper, 0, A, C, Flag);
			else {
				*Flag = OK;
				/* old interp --
				*C = A div B;
				*/
				x = A ;
				y = B ;
				*C = A / B;
				z = x / y ;
				if( (z < 0) && (z != *C))
				    *C = *C - 1 ;
					
			}
			break;
		}

	} /* end switch */
	   
}


