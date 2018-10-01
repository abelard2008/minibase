#ifndef _HASH_TBL_H
#define _HASH_TBL_H

#include <stdlib.h>
#include <ostream>

const int MEMORY = 0;
const int DISK = 1;

class hash_elem {
 public:
  void *tup_ptr;
  class hash_elem *next;

  hash_elem(){tup_ptr=NULL; next=NULL;}
  hash_elem(void *ptr){ tup_ptr = ptr; next=NULL;}
  ~hash_elem(){};  // have to define it correctly later
};


class hash_tbl {
 private:
  int _size;
  hash_elem **tbl;

 public:
  hash_tbl(int size);
  ~hash_tbl();
  hash_elem * retBucketptr(int bucket){ return tbl[bucket];}
  int insert(int i, void *t_ptr);
};

#endif
