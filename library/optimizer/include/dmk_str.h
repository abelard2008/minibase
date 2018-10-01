/////////////////////////////////////////////////////////////////////////////
// Dynamic String class
/////////////////////////////////////////////////////////////////////////////
//
//	 File: str.h
//	 Date: 11-22-93
//	  Rev: 1.03
// Author: David M. Koscinski
//
/////////////////////////////////////////////////////////////////////////////
//
// To Do:
//		Allow buffer size minimization
//		Implement create new c-string from String;
//		Implement compiler switched debugging
//		Implement sub String search methods
//		Implement regular expression search methods
//		Think of other stuff to add
//		Test, Test, Test
//
/////////////////////////////////////////////////////////////////////////////
//
//	Pitfalls:
//		When using a String as an argument to a function
//			like printf, be sure to use the () method.	This
//			this passes a const char * to the actual text.
//			For instance:
//				printf("this is a String '%s'\n", s());
//			This is necessary because no type checking is done
//			on arguments of a variable argument list function.
//
//		When using the Printf(const char *, ...) method,
//		be aware that the resulting String must be less than
//			DEFAULT_MAX_PRINTF chars long.	If there is a chance
//			that it will be longer, use the alternate Printf method
//			that allows you to explicitly state the max Length.
//
/////////////////////////////////////////////////////////////////////////////
//
// History:
//
// Rev   Date     Author  Description
//---------------------------------------------------------------------------
// v1.05 5/5/94   sch     Added two new operators for converting longs and
//                        doubles to Strings.
//                        String LongToString(long i);
//                        String DoubleToString(double r);
//---------------------------------------------------------------------------
// v1.04 4/15/94  sch     Changed >> operator to recognize white space as a
//                          delimiter instead of just newlines
//                        Modified Compare to handle null strings w/o crashing
//---------------------------------------------------------------------------
// v1.03 11/22/93 dmk     Changed [] operator to return reference to
// 			    allow s[3] = 'g'
//			  Changed size_t to ushort
//                        Removed useless << operators
//			  Changed Trim methods to use isspace() for ws detect
//---------------------------------------------------------------------------
// v1.02 11/04/93 dmk     Added  portable strupr, strlwr
//---------------------------------------------------------------------------
// v1.01 10/01/93 dmk     Added  String operator = (char c)
//		      	  String operator = (const char * cstr)
//                        void Clean()
//---------------------------------------------------------------------------
// v1.00 09/17/93 dmk     Initial release
//
/////////////////////////////////////////////////////////////////////////////


#ifndef STR_H
#define STR_H

#define DEFAULT_MAX_PRINTF	256 	// default maxLen for Printf(format, ...)
#define DEFAULT_ALLOCINCR	8		// default allocation increment

class String;
class DAofstream;

#include <stddef.h>
#include <ostream>
#include "da_types.h"


class String
{
public:
	enum StrError { ERR_ALLOC, ERR_TOO_LONG, ERR_INVALID, ERR_PRINTF, ERR_RANGE };

	// constructor
	String();

	String(const String & str);
	String(const char * cstr);
	String(char ch);

	String(ushort count, char fillCh = '\0');

	// destructor
	~String();

	// reduce a string to ""
	void Clean();

	// value return methods
	ushort Length() const;
	ushort Size() const;

	// create a c-string from String method
	const char * operator () () const;
	const char * operator () (ushort pos) const;
	operator const char * () const;

	// assignment methods
	String operator = (const String & str);
	String operator = (const char * cstr);
	String operator = (char ch);

	// conversion methods
	friend String LongToString(long i);
	friend String DoubleToString(double r);

	// concatenation methods
	friend String operator + (const String & str1, const String & str2);
	friend String operator + (const String & str1, char ch);

	void operator += (const String & str);
	void operator += (char ch);

	// comparison methods
	int operator <  (const String & str) const;
	int operator >  (const String & str) const;
	int operator <= (const String & str) const;
	int operator >= (const String & str) const;
	int operator == (const String & str) const;
	int operator != (const String & str) const;

	int Compare(const String & str, int caseChk = False) const;

	// substring deletion method
	void Delete(ushort pos, ushort count);

	// substring insertion methods
	void Insert(ushort pos, char ch);
	void Insert(ushort pos, const String & str);

	// substring retrieval method
	String Mid(ushort start, ushort count) const;
	String Left(ushort count) const;
	String Right(ushort count) const;

	// justify methods
	void LJustify(ushort len, char sym = ' ');
	void RJustify(ushort len, char sym = ' ');
	void CJustify(ushort len, char sym = ' ');

	// trim methods (removes whitespace from front|back|both sides)
	void Trim();
	void LTrim();
	void RTrim();

	// format
	void Printf(const char * format, ...); // maxLen < 256
	void Printf(ushort maxLen, const char * format, ...);

	// character retrieval method
	char & operator [] (ushort pos) const;

	// case-modification methods
	void ToUpper();
	void ToLower();

	String AsUpper() const;
	String AsLower() const;

	// stream input/output methods
	friend istream & operator >> (istream & strm, String & str);

protected:
	// instance variables
	ushort Siz; // allocated size
	ushort Len; // current length
	char * Txt; // pointer to text

private:
	// class constant
	static ushort AllocIncr;

	// pointer to error handler
	static void ErrorHandler(StrError err);

	// calc alloc size for needed bytes
	static ushort CalcSiz(ushort needed);

	// temporary buffer used in Printf(format, ...) method
	static char str_buffer[DEFAULT_MAX_PRINTF];
};


// destructor
inline String::~String()
{
	if( Txt ) delete [] Txt;
}

// clean a string
inline void String::Clean()
{
	if( Txt ) delete [] Txt;

	Siz = 0;
	Len = 0;
	Txt = NULL;
}

// value return methods
inline ushort String::Length() const
{
	return Len;
}

inline ushort String::Size() const
{
	return Siz;
}

// add-assignment operator
inline void String::operator += (const String & str)
{
	*this = *this + str;
}

inline void String::operator += (char ch)
{
	*this = *this + ch;
}

// create a c-string from String method
inline String::operator const char * () const
{
	return Txt;
}

inline const char * String::operator () () const
{
	return Txt;
}

inline const char * String::operator () (ushort pos) const
{
	if( pos >= Len )
		ErrorHandler(ERR_INVALID);

	return &Txt[pos];
}

// comparison methods
inline int String::operator <  (const String & str) const
{
	return (Compare(str) < 0);
}

inline int String::operator >  (const String & str) const
{
	return (Compare(str) > 0);
}

inline int String::operator <= (const String & str) const
{
	return (Compare(str) <= 0);
}

inline int String::operator >= (const String & str) const
{
	return (Compare(str) >= 0);
}

inline int String::operator == (const String & str) const
{
	return (Compare(str) == 0);
}

inline int String::operator != (const String & str) const
{
	return (Compare(str) != 0);
}

// character retrieval method
inline char & String::operator [] (ushort pos) const
{
	if( pos >= Len )
		ErrorHandler(ERR_RANGE);
	return Txt[pos];
}

#endif
