#include "StringUtils.h"



std::string LongToString(long i)
{
  std::string result; 
  long j, k;  

  if (i < 0) {
    result += '-';
    i = -1 * i;
  }
  for (j = 1; j * 10 <= i; j = j * 10);
  for ( ; j > 0; j = j / 10) {
    for (k = 0; i >= j; k++, i -= j);
    result += ('0' + k);
  }
  return result;
}


std::string DoubleToString(double r)
{
  std::string result; 
  double j;
  long exponent;
  char k;     

  if (r < 0) {
    result += '-';
    r = -1 * r;
  }
  
  if (r > 100000) 
    for (exponent = 0; r >= 10; exponent++, r = r / 10);
  else
    exponent = 0;

  for (j = 1; j * 10 <= r; j = j * 10);
  for ( ; j > 0.9 || r > 0.00001; j = j / 10) {
    for (k = 0; r >= j; k++, r -= j);
    result += ('0' + k);
    if (j > 0.9 && j < 1.1 && r > 0)
      result += '.';
  }
  
  if (exponent > 0) {
    result += "e+";
    result += LongToString(exponent);
  }

  return result;
}

