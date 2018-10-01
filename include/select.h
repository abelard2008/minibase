#ifndef __SELECT__
#define __SELECT__


class Select : public Iterator
{
 private:
   AttrType * _in;
   int       _len_in;
   Iterator* _am;
   PredList* OutputFilter;

 public:
   Select(AttrType in[],			// Array containing field types of R.
	  int     len_in,		// # of columns in R.

	  Iterator *  am,		// access method for input to select
	  PredList * outFilter		// Ptr to a predicate list
         );

   Stataus get_next(char * &tuple);	// The tuple is returned.

   ~Select();
};

class IndexSelect : public Iterator
{
 private:
   // Irrelevant

 public:
   IndexSelect(AttrType  in[],		// Array containing field types
	       int      len_in,		// # of columns in 

	       int      indexCol,	// The column on which the index is on.
	       Value    lowerBound,	// The lower bound
	       Value    upperBound,	// The upper bound
	       Operator op1,		// op1 is either < or <=
	       Operator op2,		//  and so is op2. Together they capture all
	       				//  ALL index conditions.
	       char   * indexName,	// Name of the index file.
	       char   * relationName,
	       PredList *outFilter
	      )

   Status get_next(char *&tuple);
   ~IndexSelect();

};
#endif

