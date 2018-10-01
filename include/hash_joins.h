
#ifndef __HASH_JOINS__
#define __HASH_JOINS__

#include <unistd.h>
#include "hash_tbl.h"
#include "iterator.h"
#include "io_bufs.h"
#include "fileIspoof.h"
#ifndef TEST
  #include "heapfile.h"
#endif

// ===========================================================================

//
// This file contains the interface for the joins.
// We name the two relations being joined as R and S.
//

// ===========================================================================



class simple_hash : public Iterator
{
private:
  AttrType *_in1;    	
  int in1_len;
  AttrType *_in2;
  int in2_len;
  int jc_in1;              // join column for am1 (R)
  int jc_in2;              // join column for am2 (S)
  int _n_pages;            // number of buffer pages
  Iterator *p_i1;      
  Iterator *p_i2;
  int _Rpages;             // number of R pages
  int _Spages;             // number of R pages
  FldSpec *perm_mat;       // matrix used for projection
  CondExpr ** OutputFilter;// used for selection
  int nOutFlds;
  int R_start;            // # of R tuples  at the beginning of pass i
  int R_start_tmp;        // had to store the previous R_start in a tmp var  
  int S_start;            // # of S tuples at the start of pass i
  hash_tbl *h_table;      // pointer to hash table
  int HSize;              // size of hash table
  int temp_file_fd1, temp_file_fd2, temp_file_fd3, temp_file_fd4 ;
  AttrType sortFldType;   // name is confusing. This is just the attrType
			  // for the join column.
  IO_buf  *R_io_write, *S_io_write;
			  // IO_bufs used for writing 'overflow' tups to disk
  spoofI_buf  *R_i_read, *S_i_read;
			  // spoof bufs for reading from temp files
  O_buf   *bm_o_buf;      // buffer space for in-memory hash table
  Tuple    * tuple1, * tuple2;
  int        t1_size, t2_size;
  char **bufs;            // buffer manager for testing
  int PassEnd;           // indicates the end of a pass
  int firstTime;       
  int firstSpass;        // set to false once we go thro' all of S
  Tuple  *Jtuple;         // result tuple

  Tuple *last_STup;       // this is used for saving the S-tuple value
			  // from the last call to get_next.
  hash_elem *last_h_elem; // used for saving the last hash_element if we
			  // have more than one element in a bucket

  int h1(int i);     // hash-func1 : this returns either MEMORY or DISK
  int h2(int i);     // returns a bucket number(this is used if 
		     //	h1 returns MEMORY)
  int ret_bufNum(int); 
  void display_hash_tbl();

public:
    simple_hash      (AttrType in1[],      // Array containing field types of R.
		      int     len_in1,     // # of columns in R.
		      short s1_sizes[], 
		      AttrType in2[],      // Array containing field types of S.
		      int     len_in2,     // # of columns in S.
		      short s2_sizes[], 
		      int     join_col1,   // The col of R to be joined with
		      int     join_col2,   // the col of S.
		      
		      int     amt_of_mem,  // INPUT buffer PAGES
		      
		      Iterator *    am1,   
		      Iterator *    am2,  
		      int Rpages,                // pages in R
		      CondExpr ** outFilter,       // Ptr to the output filter
		      FldSpec *proj_list,
		      int n_out_flds,
		      Status & s
		      );


  Status get_next(Tuple * &tuple);           // The next tuple is returned.

   ~simple_hash();
};





//=======================================================================


class grace_hash : public Iterator
{
private:
  AttrType *_in1;
  int in1_len;
  AttrType *_in2;
  int in2_len;
  int jc_in1;   	// join column for am1 (R)
  int jc_in2;		// join column for am2 (S)
  int _n_pages;       	// number of buffer pages
  Iterator *p_i1;
  Iterator *p_i2;
  FldSpec *perm_mat;   	// matrix used for projection
  CondExpr ** OutputFilter; // used for selections
  int nOutFlds;
  int n_runs;		// number of runs for the grace join
  int runSizeInTups;   	// average size of the R-runs
  int run_num;
  hash_tbl *h_table;
  int *Rtemp_files;  	// array of R temp files
  int *Stemp_files;  	// array of S temp files
  AttrType sortFldType;
  O_buf  **R_o_bufs;
  O_buf  **S_o_bufs;    // bufs used for writing runs to temp files
  O_buf   *bm_o_buf;    // buffer space for in-memory hash table for each run
  spoofI_buf   *R_i_buf;
  spoofI_buf   *S_i_buf;// used for reading in the temp files for each run 
  int *Rn_tuples;       // array for storing the number of R tuples in each run
  int *Sn_tuples;       // array for storing the number of S tuples in each run
  Tuple    * tuple1, * tuple2;
  int        t1_size, t2_size;
  char **bufs;       // buffer manager
  int PassEnd;
  Tuple  *Jtuple;

  Tuple *last_STup;
  hash_elem *last_h_elem;

  int h1(int i){ return i%n_runs;}   // hash-func1 : 0 to n_runs-1
  int h2(int i) { return i% runSizeInTups;}  // used to get a hash-bucket
  int ret_bufNum(int);
  void display_hash_tbl();

public:
    grace_hash      (AttrType in1[],       // Array containing field types of R.
		      int     len_in1,     // # of columns in R.
		      short s1_sizes[], 
		      AttrType in2[],      // Array containing field types of S.
		      int     len_in2,     // # of columns in S.
		      short s2_sizes[], 
		      int     join_col1,   // The col of R to be joined with
		      int     join_col2,   // the col of S.
		      
		      int     amt_of_mem,  // INPUT buffer PAGES
		      
		      Iterator *    am1,   
		      Iterator *    am2,        
		      int Rpages,      //  pages in R
		      CondExpr ** outFilter,  
		      FldSpec *proj_list,
		      int n_out_flds,
		      Status &s
		      );


  Status get_next(Tuple * &tuple);           // The next tuple is returned.


   ~grace_hash();
};




//======================================================================



class hybrid_hash : public Iterator
{
private:
  AttrType *_in1;
  int in1_len;
  AttrType *_in2;
  int in2_len;
  int jc_in1;		// join column for am1 (R)
  int jc_in2;		// join column for am2 (S)
  int _n_pages;		// number of buffer pages
  Iterator *p_i1;
  Iterator *p_i2;
  int _Rpages;    	// number of R pages
  int R0_size;  	// size of R0 run in pages
  FldSpec *perm_mat;    // projection matrix
  int nOutFlds;
  CondExpr ** OutputFilter;    // selection condition
  int n_runs;    	// these are the runs other than run0
			// actually, non-Zero runs are again
			// number from 0 upwards(for coding convenience) 
  int runSizeInTups;   	// average size of R-runs
  int run_num;        	// 0 to n_runs-1; 0 is repeated for convenience
  hash_tbl *h_table;
  int *Rtemp_files;  	// array of R temp files
  int *Stemp_files;  	// array of S temp files
  AttrType sortFldType;
  O_buf  **R_o_bufs;    // array of output bufs for secondary runs
  O_buf  **S_o_bufs;
  O_buf   *bm_o_buf;    // buffer space for in-memory hash table
  spoofI_buf   *R_i_buf;// array of input bufs for reading in temp files
  spoofI_buf   *S_i_buf;
  int *Rn_tuples;     	// number of R-tuples in each run
  int *Sn_tuples;
  Tuple    * tuple1, * tuple2;
  int        t1_size, t2_size;
  char **bufs;      	// buffer manager for testing
  int firstSpass;
  int PassEnd;
  Tuple  *Jtuple;

  Tuple *last_STup;
  hash_elem *last_h_elem;

  int h1(int i) {          // this returns disk or memory
    if(i%_Rpages < R0_size) 
	  return MEMORY;
    else
	  return DISK;
  }
  int h2(int i){ return i%n_runs;}   // used for getting a run number 
  int h3(int i) {		     // used for picking a hash bucket 
    if(firstSpass)
  	return i% ( R0_size * (MINIBASE_PAGESIZE/t1_size));	
    else
	return i% runSizeInTups;
   }

  void display_hash_tbl();


public:
    hybrid_hash      (AttrType in1[],      // Array containing field types of R.
		      int     len_in1,     // # of columns in R.
		      short s1_sizes[], 
		      AttrType in2[],      // Array containing field types of S.
		      int     len_in2,     // # of columns in S.
		      short s2_sizes[], 
		      int     join_col1,   // The col of R to be joined with
		      int     join_col2,   // the col of S.
		      
		      int     amt_of_mem,  // INput buffer PAGES
		      
		      Iterator *    am1,     
		      Iterator *    am2,    
		      int Rpages,                // pages in R
		      CondExpr ** outFilter,     // Ptr to the output filter
		      FldSpec *proj_list,
		      int n_out_flds,
		      Status &s
		      );


  Status get_next(Tuple * &tuple);           // The next tuple is returned.

   ~hybrid_hash();
};


#endif
