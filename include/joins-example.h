#ifndef __JOINS_EXAMPLE__
#define __JOINS_EXAMPLE__

#include "iterator.h"



enum Reln {sailors, boats, reserves};

class RelnScan : public Iterator {
 private:
   Reln _reln;
   int  index;
   int  count;
   Tuple* o_tuple;

   void ConstructSailorsTuple(Tuple *&t);
   void ConstructBoatsTuple(Tuple *&t);                                       
   void ConstructReservesTuple(Tuple *&t);
 public:
   RelnScan(Reln reln, AttrType in[], int n_types);
   RelnScan(char * reln, AttrType in[], int n_types);
   ~RelnScan();
   Status get_next(Tuple *& tuple);
};


void Query1(void);
void Query2(void);
void Query3(void);
void Query4(void);
void Query5(void);
void ConstructSailorsTuple(Tuple *&t);
void ConstructBoatsTuple(Tuple *&t);
void ConstructReservesTuple(Tuple *&t);
void Query1_CondExpr(CondExpr **&expr);
void Query2_CondExpr(CondExpr **&expr, CondExpr **&expr2);
void Query3_CondExpr(CondExpr **&expr);
void Disclaimer(void);

#endif

