// RTPage.C
//
// Armand Zakarian
// May 1995
//
//
// Provides implementation for RTPage::print()
// Used by the implementation of an R-tree access method for Minirel

#include <ostream>

#include "RTPage.h"


void RTPage::print() 
{
    std::cout<<"----------------------------------------------\n" ;
    std::cout<<"Page: "<<page<<", Pagetype: "<<(int)pagetype
        <<", Nslots: "<<nslots<<", Keysize: "<<keysize<<'\n' ;
    
    int dim=keysize/(2*sizeof(float)) ;
    RTPageScan *scan=new_scan() ;
    char *b ;
    RID rid ;
    typedef float (*Box)[2] ;
    while(scan->next(b, rid)) {
      Box box=(Box)b ;
      for(int i=0 ; i<dim ; i++) {
        std::cout<<"["<<box[i][0]<<", "<<box[i][1]<<"] " ;
      }
      std::cout<<"   RID:("<<rid.pageNo<<", "<<rid.slotNo<<")\n" ;
    }
    std::cout<<"----------------------------------------------\n" ;
    delete scan ;
}


