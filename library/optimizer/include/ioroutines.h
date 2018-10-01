// ioroutines.h
// input/output routines for a few of the types defined in query.h
//
// 

// the reason for the seperation is that these aren't necessarily the most
// general output functions -- these are specifically for output
// to a bogus catalog or to a GUI frontend.


#ifndef IOROUTINES_H
#define IOROUTINES_H

#include <ostream>
#include <fstream>
#include "query.h"

std::ifstream& operator>> (std::ifstream &infile, AttrType &attr);
std::ifstream& operator>> (std::ifstream &infile, TupleOrder &tupleOrder);
std::ofstream& operator<< (std::ofstream &outfile, AttrType attr);
std::ofstream& operator<< (std::ofstream &outfile, TupleOrder tupleorder);

#endif
