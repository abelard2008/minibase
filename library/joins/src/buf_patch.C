
#ifndef TEST

#include "iterator.h"
#include "buf.h"
#include "new_error.h"


// the functions here are used to interface with the BufferMng.
// Add the following 2 functions to the
// private part of the Iterator base class OR to every join class

extern SystemDefs* minibase_globals;

// tries to get n_pages of buffer space
Status Iterator::get_buffer_pages(int n_pages, int *PageIds, char **bufs) {
  Status s;
  Page* pgptr;        // added by BK
  for(int i=0; i < n_pages; i++) {
    pgptr = (Page*)bufs[i];
    s= MINIBASE_BM->newPage(PageIds[i], pgptr);
    bufs[i] = (char*)pgptr;
    if(s!=OK)
        return s;
  }
  return OK;
}



// free all the buffer pages we requested earlier.
// should be called in the destructor

Status Iterator::free_buffer_pages(int n_pages, int *PageIds ) {
  Status s;
  for(int i=0; i < n_pages; i++) {
    s= MINIBASE_BM->freePage(PageIds[i]);
    if(s!=OK)
        return s;
  }
  return OK;
}

#endif
