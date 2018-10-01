#include <ostream>
#include <string.h>
#include <assert.h>
//#include "test.h"
#include "new_error.h"
#include "tuple.h"

int Tuple::max_size = 1024;

Status Tuple::setHdr(short numFlds, AttrType types[], short strSizes[]){ 

// setHdr will set the header of this tuple. If there is enough
// space available returned status will be OK. 
// Array strSizes contains the sizes of the string (excluding trailing \0)
// Array types contains the types that will be in this tuple.

	if((numFlds + 2) * sizeof(short) > (unsigned)max_size){
		return TUPLE_TOO_BIG;
	}
	fldCnt = numFlds;
	short strCount = 0;
	short incr;
	fldOffset[0] = pad((numFlds + 2) * sizeof(short), types[0]);
        int i;
	for( i = 1; i < numFlds; i++){
		switch(types[i - 1]){
		case attrString:
			incr = strSizes[strCount] + 1;
			strCount++;
			break;
		case attrInteger:
			incr = sizeof(int);
			break;
		case attrReal:
			incr = sizeof(float);
			break;
		default:
			return TUPLE_TYPE;
		}
		fldOffset[i] = pad(fldOffset[i - 1] + incr, types[i]);
	}
	switch(types[numFlds - 1]){
	case attrString:
		incr = strSizes[strCount] + 1;
		break;
	case attrInteger:
		incr = sizeof(int);
		break;
	case attrReal:
		incr = sizeof(float);
		break;
	default:
		return TUPLE_TYPE;
	}
	fldOffset[numFlds] = fldOffset[i - 1] + incr;
	if(fldOffset[numFlds] > max_size){
		return TUPLE_TOO_BIG;
	}
	return OK;
}

void Tuple::print(AttrType type[], std::ostream& out){ 

	int   i, temp_i;
	float temp_f;

	// Print this tuple to stream out, by default std::cout; 

	out << "[";
	for(i = 0; i < fldCnt - 1; i++){
		switch(type[i]){
		case attrInteger:
			memcpy((char *) &temp_i, (char *) this + fldOffset[i], sizeof(int));
			out << temp_i;  // *((int*)((char*)this + fldOffset[i]));
			break;
		case attrReal:
			memcpy((char *) &temp_f, (char *) this + fldOffset[i], sizeof(float));
			out << temp_f;  // *((float*)((char*)this + fldOffset[i]));
			break;
		case attrString:
			out << (char*)this + fldOffset[i];
			break;
		case attrNull: case attrSymbol: break;
		}
		out << ", ";
	}
	switch(type[fldCnt - 1]){
	case attrInteger:
		memcpy((char *) &temp_i, (char *) this + fldOffset[i], sizeof(int));
		out << temp_i;  // *((int*)((char*)this + fldOffset[fldCnt - 1]));
		break;
	case attrReal:
		memcpy((char *) &temp_f, (char *) this + fldOffset[i], sizeof(float));
		out << temp_f;   // *((float*)((char*)this + fldOffset[fldCnt - 1]));
		break;
	case attrString:
		out << (char*)this + fldOffset[fldCnt - 1];
		break;
	case attrNull: case attrSymbol: break;
	}
	out << "]\n";
}

short Tuple::pad(short offset, AttrType type)
{
// Padding must be used when storing different types.
    switch ( type )
      {
	default:
            return offset;
	case attrInteger:
            return (offset + sizeof(int) - 1) / sizeof(int) * sizeof(int);
	case attrReal:
            return (offset + sizeof(float) - 1) / sizeof(float) * sizeof(float);
      }
}

