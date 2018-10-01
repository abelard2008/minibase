/*
 * IO routines for catalog members.
 *
 */

#include "ioroutines.h"

std::ifstream &operator>> (std::ifstream &infile, AttrType &attr)
{
    char typechar;
    infile >> std::ws;
    infile >> typechar;
  switch (typechar)
    {
    case 'R': case 'r':
      attr = attrReal;
      break;

    case 'I': case 'i':
      attr = attrInteger;
      break;

    case 'T': case 't':
      attr = attrString;
      break;
      
    default:
	  attr = attrNull;
      break;
    }
  return infile;
}

std::ifstream &operator>> (std::ifstream &infile, TupleOrder &tupleOrder)
{
  char orderchar;
  infile >> std::ws;
  infile >> orderchar;
  switch (orderchar)
    {
    case 'A': case 'a':
      tupleOrder = Ascending;
      break;

    case 'D': case 'd':
      tupleOrder = Descending;
      break;

    case 'R': case 'r':
      tupleOrder = Random;
      break;

    default:
      tupleOrder = Random;
      break;
    }
  return infile;
}

std::ofstream &operator<< (std::ofstream &outfile, AttrType attr)
{
    switch ( attr )
    {
      case attrString:
	outfile << "T ";		
	break;
	
      case attrInteger:
	outfile << "I ";
	break;
	
      case attrReal:
	outfile << "R ";
	break;
      default:
	
	outfile << "N "; // null
	break;
    }
    return outfile;
    
}

std::ofstream& operator<< (std::ofstream &outfile, TupleOrder tupleorder)
{
    switch ( tupleorder )
    {
      case Ascending:
	outfile << " A ";
	break;
      case Descending:
	outfile << " D ";
	break;
      case Random:
	outfile << " R ";
	break;
      default:
	outfile << " ? ";
	break;
    }
    return outfile;    
}





