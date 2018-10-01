//
// da_types.cpp
//

#include <stdlib.h>
#include <iostream>

#include "da_types.h"

void FatalError(ErrorCode ec)
{
    std::cerr << "Fatal Error: ";
    switch( ec ) 
	{
	case RANGE:
	    std::cerr << "RANGE VIOLATION";
	    break;
	case MEM:
	    std::cerr << "OUT OF MEMORY";
	    break;
	case NULLPTR:
	    std::cerr << "NULL POINTER ASSIGNMENT";
	    break;
	case SIZE:
	    std::cerr << "ARRAY SIZE EXCEEDS 64K";
	    break;
	case COPY: 
	    std::cerr << "ARRAY COPY SIZE MISMATCH";
	    break;
	default:
	    std::cerr << "UNDEFINED";
	    break;
	}
    std::cerr << std::endl;
//    exit(ec);
}
