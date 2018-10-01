// R-tree index for Minirel
// A Zakarian

 
class RtreeScan ;
 
class RtreeFile {
  friend RtreeScan ;

public:
  RtreeFile(Status &status, const char *name, int dim =2, int m =2) ;
   	// name is filename on disk
  	// dim is dimensionality of indexed objects (only when index is created)
	// m is min occupancy of a page                       --- / ----
        // if index exists, open it; else create it
  ~RtreeFile() ;
        // closes index
 
  Status insert_record(const void *box, const RID recid) ; 
			// insert recid with bounding box 
  Status delete_record(const void *box, const RID recid) ;
			// delete leaf entry recid given its Box

  IndexScan *new_scan(const void *box) ;  // create a scan for a given Box

  typedef float Box[][2] ;

  // ``box'' arguments in insert, delete and new_scan will be treated
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

private:
} ;


// given a box, return RIDs (into main relation) of objects
// that intersect this box
// notice that constructor is private;  RtreeScan's are
// created by RtreeFile::new_scan()

class RtreeScan: public IndexScan {
  friend RtreeFile ;

public:
  Status next_record(RID &recid) ;
  Status delete_current_record() ;
} ;


