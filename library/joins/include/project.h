#ifndef __PROJECT__
#define __PROJECT__


class project : public Iterator
{
 private:
	// Irrelevent

 public:
   project(AttrType in[],		// Array containing field types of R.
	   int     len_in,		// # of columns in R.

	   int     permutation_array[], // array of permutations. If a field i is filtered out,
					// the value p_a[i] = -1. Else, if field i is in
					// position j in the output, p_a[i] = j.
	   Iterator *    am		// access method for left input to join.
         );

   ProjectError get_next_tuple(char * &tuple);     // The tuple is returned.

   void perror(ProjectError error_code);// Prints an error message corr. to error_code

   ~project();
};



#endif

