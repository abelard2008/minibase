#include <ostream>
#include <string.h>
#include <assert.h>
#include "error.h"
#include "tuple.h"

int Tuple::max_size = 1024;

Status Tuple::setHdr(short numFlds, AttrType types[], short strSizes[]){ 

// setHdr will set the header of this tuple. If there is enough
// space available returned status will be OK. 
// Array strSizes contains the sizes of the string (excluding trailing \0)
// Array types contains the types that will be in this tuple.

	if((numFlds + 2) * sizeof(short) > max_size){
		return TUPLE_TOO_BIG;
	}
	fldCnt = numFlds;
	short strCount = 0;
	short incr;
	fldOffset[0] = pad((numFlds + 2) * sizeof(short), types[0]);
	for(int i = 1; i < numFlds; i++){
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
	fldOffset[numFlds] = pad(fldOffset[i - 1] + incr, attrInteger);
	if(fldOffset[numFlds] > max_size){
		return TUPLE_TOO_BIG;
	}
	return OK;
}

void Tuple::print(AttrType type[], std::ostream& out){ 

	// Print this tuple to stream out, by default std::cout; 

	out << "{";
	for(int i = 0; i < fldCnt - 1; i++){
		switch(type[i]){
		case attrInteger:
			out << *((int*)((char*)this + fldOffset[i]));
			break;
		case attrReal:
			out << *((float*)((char*)this + fldOffset[i]));
			break;
		case attrString:
			out << (char*)this + fldOffset[i];
			break;
		}
		out << ", ";
	}
	switch(type[fldCnt - 1]){
	case attrInteger:
		out << *((int*)((char*)this + fldOffset[fldCnt - 1]));
		break;
	case attrReal:
		out << *((float*)((char*)this + fldOffset[fldCnt - 1]));
		break;
	case attrString:
		out << (char*)this + fldOffset[fldCnt - 1];
		break;
	}
	out << "}\n";
}

short Tuple::pad(short offset, AttrType type){

// Padding must be used when storing different types.

	switch(type){
	case attrString:
		return offset;
	case attrInteger:
		short sizeofint = sizeof(int);
		if(offset % sizeofint == 0){
			return offset;
		}
		else{
			return (offset / sizeofint + 1) *  sizeofint;
		}
	case attrReal:
		short sizeofflt = sizeof(float);
		if(offset % sizeofflt == 0){
			return offset;
		}
		else{
			return (offset / sizeofflt + 1) *  sizeofflt;
		}
	}
}
