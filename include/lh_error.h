// linearhashing error handling in MINIREL
// Weiqing Huang, whuang@cs.wisc.edu, May 1995

// $Id: lh_error.h,v 1.2 1996/03/31 18:26:51 luke Exp $

#ifndef _LH_ERROR_H_
#define _LH_ERROR_H_

#include "new_error.h"

// error code in linearhashing
enum LinearhashingErrCodes {
    BADFILENAME,
    BADKEYPARM,
    NOPAGESPACE,
    INDEXDESTROYED,
};


    
#endif
