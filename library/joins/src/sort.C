// ==============================================================================
//
// Sort.C
//	This file contains an implementation of an external sorting routine.
// The sorting routine uses the "tournament-sort" routine as described in
// Shapiro's paper. (Shapiro uses it to sort for the sort-merge join)
//
// ==============================================================================

//#include "test.h"
//#include "example.h"
#include "tuple_utils.h"
#include "io_bufs.h"
#include "sort.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>

#define ARBIT_RUNS		10


// ==============================================================================
//
// Sort - This is the constructor for the class. It takes the fields types of
//        its input, a pointer to an iterator, the field of the input to sort
//        on, the sort order (Ascending or descending), and the number of buffer
//        pages available for sorting.
//
// ==============================================================================

//
// Internal functions
//

void MIN_VAL(Tuple * &lastElem, AttrType sortFldType, int sortFldLen);
void MAX_VAL(Tuple * &lastElem, AttrType sortFldType, int sortFldLen);
void expand(int * &temp_files, int &n_temp_files);
void expand(HeapFile ** &temp_files, int &n_tempfiles);
void swap(pnodeSplayPQ * & x, pnodeSplayPQ * & y);
void swap(int *& x, int * &y);

/**************************************************************
Comments put here by BK

PROBLEM => Where does max_elems come from????  It is 
arbitrarily set to 500..

The sort iterator takes tuple information, the input iterator,
and the number of pages required for the sort as arguments.

It reserves n_pages from the buffer manager and creates 
temporary heap files to store tuples in.  The number of
heap files depends on the number of runs.

get_buffer_pages()
temp_files[] = new HeapFile(NULL, s);

// o_buf - output buffer to a file
// used for dumping tuples into a heapfile
o_buf.init(bufs, _n_pages, tuple_size, temp_files[0]);

**************************************************************/
Sort::Sort(AttrType  in[],
	   int       len_in,
	   short     str_sizes[],
	   Iterator *am,			// the iterator input to be sorted.
	   int       sort_fld,			// Number of the sort field
	   TupleOrder sort_order,		// the sort order
	   int       sort_fld_len,		// length of the sort field.
	   int       n_pages,
	   Status   & s)
{
   s = OK;
   _in       = new AttrType[len_in];
   assert(_in);
   n_cols    = len_in;
   for (int i = 0; i < len_in; i++)
      _in[i] = in[i];

   int i, n_strs = 0;
   for (i = 0; i < len_in; i++)  if (_in[i] == attrString) n_strs++;

   str_lens = new short [n_strs];
   assert(str_lens);
   for ( i = 0, n_strs = 0; i < len_in; i++)
   {
      if (_in[i] == attrString)
      {
	 str_lens[n_strs] = str_sizes[n_strs];
	 n_strs++;
      }
   }

   Tuple *t = (Tuple *) new char [Tuple::max_size];
   assert(t);
   t->setHdr(len_in, _in, str_sizes);
   tuple_size = t->size();

   delete [] t;

   _am       = am;
   _sort_fld = sort_fld;
   order     = sort_order;
   _n_pages  = n_pages;

   bufs = new char *[_n_pages];
   assert(bufs);

   bufs_pids = new int[_n_pages];       // added by BK
   assert(bufs_pids);

#ifdef TEST
   for (int count = 0; count < _n_pages; count++)
   {
      bufs[count] = new char [MINIBASE_PAGESIZE];
      assert(bufs[count]);
   }
#else
   // Whatever has to be done to call the buffer manager and get pages
   get_buffer_pages(_n_pages, bufs_pids, bufs);            // modified by BK

#endif

   first_time  = TRUE;

   // As a heuristic, we set the number of runs to an arbitrary value of ARBIT_RUNS
#ifdef TEST
   temp_files  = new int [ARBIT_RUNS];
#else
   temp_files  = new HeapFile*[ARBIT_RUNS];
#endif
   n_tempfiles = ARBIT_RUNS;
   n_tuples    = new int [ARBIT_RUNS];
   n_runs      = ARBIT_RUNS;

#ifdef TEST
   temp_files[0] = open(temp_file_name(), O_RDWR);
#else
   temp_files[0] = new HeapFile(NULL, s);
   // open just one file...
#endif

   o_buf.init(bufs, _n_pages, tuple_size, temp_files[0]);
   output_tuple = NULL;		// a delete on this should not produce any problems.

#ifdef TEST
   // The priority queue implementation does not buffer pages...
   // but uses virtual memory to store the elements
   max_elems_in_heap = 500;
#else
   max_elems_in_heap = 500;
   // error("set MAX_ELEMS_IN_HEAP");
#endif

   sortFldLen = sort_fld_len;

   Q = new pnodeSplayPQ(sort_fld, in[sort_fld]);
   op_buf = (Tuple *) new char [Tuple::max_size];
   assert(Q && op_buf);
   op_buf->setHdr(n_cols, _in, str_lens);
}


Sort::~Sort()
{
#ifdef TEST
   // Free the allocated memory
   for (int count = 0; count < _n_pages; count++)
      delete [] bufs[count];
#else
   // Whatever has to be done to call the buffer manager and return the pages.
   // Because of a Kludge, we do not have to free buffer pages.
   free_buffer_pages(_n_pages, bufs_pids);

   delete temp_files[0];
   //delete [] temp_files;
#endif
   delete [] bufs;
   delete [] bufs_pids;     // added by BK
   delete [] temp_files;
   delete [] _in;
   delete [] str_lens;
   delete [] (char *) op_buf;
}



// =====================================================================================
//
// generate_runs - generates runs for the sort-merge join using the tournament tree
//                    method.
//		max_elems - max # of elements in the heap.
//

int Sort::generate_runs(int max_elems, AttrType sortFldType, int sortFldLen, Status &s)
{
   Tuple *tuple = NULL;
   pnode node_ptr;
   pnodeSplayPQ Q1(_sort_fld, sortFldType, order),
                Q2(_sort_fld, sortFldType, order),
                *pcurr_Q, *potherQ;

   Tuple * lastElem = (Tuple *) new char[Tuple::max_size];
   lastElem->setHdr(n_cols, _in, str_lens);

   int    run_num = 0;			// Keeps track of the number of runs

   int    nelems_Q1 = 0, nelems_Q2 = 0;	// # of elements in Q1 and Q2 respectively
   int  * p_elems_curr_Q = &nelems_Q1;	// ptr to # of elements in the current queue.
   int  * p_elems_otherQ = &nelems_Q2;

   int comp_res;

   pcurr_Q = &Q1;
   potherQ = &Q2;

   // Set the lastElem to be the minimum value for the sort field
   if (order == Ascending) MIN_VAL(lastElem, sortFldType, sortFldLen);
   else                    MAX_VAL(lastElem, sortFldType, sortFldLen);

   // Since the min-heap implemented by the gnu library can expand to any size,
   // we maintain a fixed maximum number of elements in the heap.
   while (nelems_Q1 + nelems_Q2 < max_elems)
   {
      if ((s = _am->get_next(tuple)) == DONE)
	 break;
      else if (s != OK) return s;

      node_ptr = new node;
      node_ptr->tuple = (Tuple *) new char [tuple->size()];
      memcpy((char *) node_ptr->tuple, (char *) tuple, tuple->size());
      pcurr_Q->enq(node_ptr);
      (*p_elems_curr_Q)++;
   }

   while (1)
   {
      node_ptr = pcurr_Q->deq();
      (*p_elems_curr_Q)--;
 
      comp_res = CompareTupleWithValue(sortFldType, node_ptr->tuple, _sort_fld, lastElem);

      if ((comp_res < 0 && order == Ascending) || (comp_res > 0 && order == Descending))
      {
	 potherQ->enq(node_ptr);	// insert deleted element into the other queue
	 (*p_elems_otherQ)++;
      }
      else
      {
	 SetValue(lastElem, node_ptr->tuple, _sort_fld, sortFldType);

	 // Write tuple to output file
	 o_buf.Put((char *) node_ptr->tuple);

	 delete [] (char *) node_ptr->tuple;
	 delete node_ptr;
      }
	 
      // Is the heap filled with smaller values
      if ((*p_elems_otherQ) == max_elems)	 // Start next run
      {
 	 // Close file for prev run and Open a new file.
	 n_tuples[run_num] = o_buf.flush();
	 run_num++;
#ifdef TEST
	 // Reinitialize the output buffer to write to a new file...
	 temp_files[run_num] = open(temp_file_name(), O_RDWR);
#else
	 temp_files[run_num] = new HeapFile(NULL, s);
	 if (s != OK) return s;
#endif
	 o_buf.init(bufs, _n_pages, tuple_size, temp_files[run_num]);

	 // Set the lastElem to be the minimum value for the sort field
	 if (order == Ascending) MIN_VAL(lastElem, sortFldType, sortFldLen);
	 else                    MAX_VAL(lastElem, sortFldType, sortFldLen);

	 if (run_num == n_tempfiles)		// Array exceeded ? -- double its size.
	 {
	    expand(temp_files, n_tempfiles);
	    expand(n_tuples, n_runs);
	 }
	 // Switch the current heap and the other heap.
	 swap(potherQ, pcurr_Q);              swap(p_elems_otherQ, p_elems_curr_Q); 
      }
      else if ((*p_elems_curr_Q) == 0)		// Is the current min heap empty?
      {
	 while (nelems_Q1 + nelems_Q2 < max_elems)
	 {
	    if ((s = _am->get_next(tuple)) == DONE)
	       break;
	    else if (s != OK) return s;

	    node_ptr = new node;
            node_ptr->tuple = (Tuple *) new char [tuple->size()];
	    node_ptr->tuple->setHdr(n_cols, _in, str_lens);
	    memcpy(node_ptr->tuple, (char *) tuple, tuple->size());

	    pcurr_Q->enq(node_ptr);	// Has to be changed.
	    (*p_elems_curr_Q)++;
	 }
      }

      // Check if we are done.
      if ((*p_elems_curr_Q) == 0) // Is the MinHeap empty despite our attempts to fill it?
      {
	 // Cannot break, there may be some smaller elems which will constitute a
	 // a new run.
	 if ((*p_elems_otherQ) == 0)	// Other queue is also empty?
	    break;
	 else		// a new run has to be started.
	 {
	    // Close file for prev run and Open a new file.
	    n_tuples[run_num] = o_buf.flush();
	    run_num++;
#ifdef TEST
	    // Reinitialize the output buffer to write to a new file...
	    temp_files[run_num] = open(temp_file_name(), O_RDWR);
#else
	    temp_files[run_num] = new HeapFile(NULL, s);
	    if (s != OK) return s;
#endif
	    o_buf.init(bufs, _n_pages, tuple_size, temp_files[run_num]);

	    // Set the lastElem to be the minimum value for the sort field
	    if (order == Ascending) MIN_VAL(lastElem, sortFldType, sortFldLen);
	    else                    MAX_VAL(lastElem, sortFldType, sortFldLen);

	    if (run_num == n_tempfiles)		// Array exceeded ? -- double its size.
	    {
	       expand(temp_files, n_tempfiles);
	       expand(n_tuples, n_runs);
	    }
	    // Switch the current heap and the other heap.
	    swap(potherQ, pcurr_Q);      swap(p_elems_otherQ, p_elems_curr_Q); 
	 }
      } // if
   }	// while (1)
   // Close the files -- only to reopen them?
   n_tuples[run_num] = o_buf.flush();
   run_num++;

   delete [] lastElem;
   // Must traverse the PQ and delete memory !!
   // Amit> Not needed b'cos the priority queues will be empty at the end anyway.

   return run_num;	// return the number of runs produced.
}



// =====================================================================================
//
// This function builds a priority queue on the input stream, and sets up the queue
// so that it produces the required stream of joined tuples.

void Sort::setup_for_merge(int tuple_size, int n_R_runs)
{
   Status status;
   int i;
   pnode node_ptr;

   i_buf = new spoofI_buf [n_R_runs];

   assert(n_R_runs <= _n_pages);
   // Construct the lists initially
   for (i = 0; i < n_R_runs; i++)
   {
      // open_file(temp_files[i]);
      // FOR TESTING, give one page to each run.
      #ifdef TEST
      i_buf[i].init(temp_files[i], &bufs[i], 1, tuple_size, n_tuples[i]);
      #else                                         // idef endif added by BK
      i_buf[i].init(temp_files[i], &bufs[i], 1, tuple_size, n_tuples[i], status);   
      #endif

      node_ptr          = (pnode) new  node;
      node_ptr->tuple   = (Tuple *) new char[tuple_size];
      node_ptr->run_num = i;				// setup run number

      // NO NO open a tFileScan object
      // Open a file scan object.....

      if (i_buf[i].Get(node_ptr->tuple) != DONE)	// read a tuple
	 Q->enq(node_ptr);				// enqueue it.
   }
   return;
}


// =====================================================================================

char * Sort::delete_min(void)
{
   struct node * node_ptr = NULL;
   Tuple * new_tuple, * old_tuple;

   node_ptr  = Q->deq();
   old_tuple = node_ptr->tuple;

   // Now, we have to insert a value from the corresponding
   // input buffer into the priority queue.

   if (i_buf[node_ptr->run_num].empty() != TRUE)
   {
      new_tuple = (Tuple *) new char [old_tuple->size()];
      Status s = i_buf[node_ptr->run_num].Get(new_tuple);	// something like this?

      if (s != DONE)
      {
	 node_ptr->tuple = new_tuple;
	 Q->enq(node_ptr);
      }
      else
      {
	 delete node_ptr;
      }
   }
   else
      delete node_ptr;  // free(node_ptr);

   return (char *) old_tuple;
}



Status Sort::get_next(Tuple * &tuple)
{
   Status status;

   if (first_time)			// First get_next call to sort routine.
   {
      first_time = FALSE;
      
      // Generate runs
      #ifdef TEST
      Nruns = generate_runs(max_elems_in_heap, _in[_sort_fld-1], sortFldLen);
      #else                         // ifdef endif was added by BK
      Nruns = generate_runs(max_elems_in_heap, _in[_sort_fld-1], sortFldLen, status);
      #endif

      // Setup state to perform merge of runs. Open input buffers for all the input files.
      setup_for_merge(tuple_size, Nruns);
   }

   if  (Q->empty())
      return DONE;

   output_tuple = delete_min();
   memcpy((char *) op_buf, output_tuple, tuple_size);
   delete [] output_tuple;

   tuple = op_buf;
   return OK;
}



void swap(int *& x, int * &y)
{
   int * temp;

   temp = x;
   x    = y;
   y    = temp;
}


void swap(pnodeSplayPQ * & x, pnodeSplayPQ * & y)
{
   pnodeSplayPQ * temp;

   temp = x;
   x    = y;
   y    = temp;
}


void expand(int * &temp_files, int &n_tempfiles)
{
   int *temp = new int [2 * n_tempfiles];
   int  i = 0;

   for (; i < n_tempfiles; i++)
      temp[i] = temp_files[i];
   delete [] temp_files;

   temp_files = temp;
   n_tempfiles *= 2;
}


void expand(HeapFile ** &temp_files, int &n_tempfiles)
{
   HeapFile **temp = new HeapFile *[2 * n_tempfiles];

   for (int index = 0; index < n_tempfiles; index++)
      temp[index] = temp_files[index];
   delete [] temp_files;

   temp_files = temp;
   n_tempfiles *= 2;
}



void MIN_VAL(Tuple *&lastElem, AttrType sortFldType, int /*sortFldLen*/)
{
   int   int_min   = INT_MIN;
   float float_min = FLOAT_MIN;
   short s_size[]  = {Tuple::max_size};
   AttrType junk[1];

   junk[0] = sortFldType;
   
   switch (sortFldType)
   {
    case attrInteger:
      lastElem->setHdr(1, junk, NULL);
      lastElem->setFld(1, int_min);
      break;
    case attrReal:
      lastElem->setHdr(1, junk, NULL);
      lastElem->setFld(1, float_min);
      break;
    case attrString:
      lastElem->setHdr(1, junk, s_size);
      lastElem->setFld(1, "\0");
      break;
    default:
      //error("Don't know how to handle attrSymbol, attrNull - void MIN_VAL()");
      printf("error in sort.c\n");
   }
   return;
}



void MAX_VAL(Tuple *&lastElem, AttrType sortFldType, int /*sortFldLen*/)
{
   int   int_max   = INT_MAX;
   float float_max = FLOAT_MAX;
   char  max_str[] = {char(127), '\0'};
   short s_size[]  = {Tuple::max_size};
   AttrType junk[1];

   junk[0] = sortFldType;
   
   switch (sortFldType)
   {
    case attrInteger:
      lastElem->setHdr(1, junk, NULL);
      lastElem->setFld(1, int_max);
      break;
    case attrReal:
      lastElem->setHdr(1, junk, NULL);
      lastElem->setFld(1, float_max);
      break;
    case attrString:
      lastElem->setHdr(1, junk, s_size);
      lastElem->setFld(1, max_str);
      break;
    default:
      printf("Don't know how to handle attrSymbol, attrNull-void MIN_VAL()\n");
   }

   return;
}
