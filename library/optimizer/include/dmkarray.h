// -*- mode:c++ -*-
// array.h
//


#ifndef array_h
#define array_h

#include <stdlib.h>
#include "da_types.h"


template <class T>
class Array
{
private:
    T *data;
    unsigned int size;

public:
    Array()
    {
	data = NULL;
	size = 0;
    }

    Array(unsigned long aSize)
    {
	if( sizeof(T) * aSize > 65535ul )
	    FatalError(SIZE);

	data = new T[aSize];
	if( !data )
	    FatalError(MEM);
	size = aSize;
    }

    // copy constructor added by Lee 8/13.
    Array(const Array<T> &array) : size(array.size)
    {
	data = new T[size];
	if (!data)
	    FatalError(MEM);
	for (unsigned i = 0; i < size; i++) {
	    data[i] = array[i];
	}
    }

    Array<T> &operator=(const Array<T> &array)
    {
	if (array.size != size) {
	    FatalError(COPY);
	};
	
	for (unsigned i = 0; i < size; i++) {
	    data[i] = array[i];
	}
	return *this;
    }


    ~Array()
    {
	if( data )
	    delete [] data;
    }


    
    T& operator[](long i) const
    {
	if( i<0 || i>=(long)size )
	    FatalError(RANGE);
	return data[i];
    }

    unsigned int Size() const { return size; }

    int ReSize(unsigned long aSize)  {
	if( sizeof(T) * aSize > 65535ul )
	    FatalError(SIZE);

	T *temp = new T[aSize];
	if( !temp )
	    FatalError(MEM);
	if (data) {
	    for (unsigned i = 0; i < ((aSize < size) ? aSize : size); i++) {
		temp[i] = data[i];
	    }
	}
	data = temp;
	size = aSize;
	return size;
    }
};


#endif
