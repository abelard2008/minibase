// Rtree.h
//
// Armand Zakarian
// May 1995
//
//
// Provides definition of classes RtreeFile and RtreeFileScan
// R-tree access method for Minirel
//
//

#ifndef RTREE_H
#define RTREE_H
 

#include "minirel.h"
#include "index.h"

#include "RTPage.h"

// max allowable height of R-tree
#define RTMAXHEIGHT 	10 

#define max(x, y)   	((x)<(y) ? (y) : (x))
#define min(x, y)   	((x)<(y) ? (x) : (y))

// keys in index are of type ``Box''
typedef float (*Box)[2] ;

class RtreeScan ;
 
// An object of class RtreeFile is associated with an unclustered
// R-tree index on disk
class RtreeFile: public IndexFile {
  friend RtreeScan ;

  char name[MAXINDEXNAME] ;	// index name
  int dim ;			// dimensionality of indexed objects
  int min_occ ;			// minimum number of entries per page
  int max_occ ;			// maximum number of entries that fit on a page

  // int root ;
  int header ;			// pageNo of header page
  Page *header_ptr ;		// pointer to header page in buffer pool
  int dirtyRoot ;		// 1 if root page is written to
  int destroyed ;		// 1 if index is destroyed

  Status destroy_tree(int page) ;
 	// destroy (physical) tree rooted at page by deallocating all pages

  Status split_page(RTPage *p, Box box, RID recid, RTPage *&r) ;
        // split a page in the R-tree
  void pick_seeds(RTPage *p, Box box, int &seed1, int &seed2) ;
        // pick two boxes that are far appart (as per Linear Splitting
        // algorithm for R-trees)

  Box new_box() { return new float[dim][2] ;} ;
        // allocate a temporary Box
  int intersect(Box box1, Box box2) ;
        // 1 if box1 and box2 intersect
  int same_box(Box box1, Box box2) ;
        // 1 if box1 and box2 coincide
  void compute_bbox(Box box, RTPage *r) ;
        // compute min enclosing rectangle for entries on page pointed 
        // to by r and update box
  double compute_enl(Box box, Box b) ;
        // return change in area of Box box if it is enlarged to cover Box b
  void add_to_box(Box box, Box bbox) ;
        // enlarge (minimally) Box bbox so that it covers Box box
  void copy_box(Box s, Box d) ;
        // make Box d be the same as box s

public:
  RtreeFile(Status &status, const char *n, int d =2, int m =2) ;
   	// n is filename on disk
  	// d is dimensionality of indexed objects (only when index is created)
	// m is min occupancy of a page   
        // if index exists, open it; else create it

  ~RtreeFile() ;
        // closes index
 
  Status insert(const void *box, const RID recid) ; 
			// insert recid with bounding box 
  Status Delete(const void *box, const RID recid) ;
			// delete leaf entry recid given its box

  Status destroy() ;	// destroy index

  IndexFileScan *new_scan(const void *box) ;  // create a scan for a given Box


  // ``box'' arguments in delete and new_scan will be treated
  // as shown:
  //
  // if(!box) { /* the whole universe */ }
  // else {
  //   I = (Box) box ;
  //   /*  I[0][0]     and I[0][1]     --> x1_low and x1_high,
  //       I[1][0]     and I[1][1]     --> x2_low and x2_high,
  //         ...
  //       I[dim-1][0] and I[dim-1][1] --> x_dim_low and x_dim_high.
  //   */
  // }

  Status print_root() ;			    // pretty print root page
} ;


// given a box, return RIDs (pointing to main relation) of objects
// that intersect this box
// notice that constructor is private;  RtreeScan's are
// created by RtreeFile::new_scan()

class RtreeScan: public IndexFileScan {
  friend RtreeFile ;

  RtreeFile *rtf ;		// RtreeFile being scanned
  float (*sbox)[2] ;		// Box being scanned
  RTPageScan *ps[RTMAXHEIGHT] ; // stack of RTPageScan* 's, one for every
  				// level in the R-tree
  int h ;			// stack pointer

  RtreeScan(RtreeFile *r, Box b) ;	// create scan

public:

  ~RtreeScan() ;			// close scan

  Status get_next(RID &recid) ;		// set recid to next record_id
  					// whose Box satisfies scan condition

  Status delete_current() ;		// delete current record
} ;

#endif

