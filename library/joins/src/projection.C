#include "projection.h"
#include "iterator.h"

// ------------------------------------------------------------------------
//
// Jtuple already has its setHdr called to setup its vital stats.
//

AttrType junk[] = { attrString, attrInteger, attrInteger, attrInteger };

void Join(Tuple * t1, AttrType type1[], int /*n_types1*/,
	  Tuple * t2, AttrType type2[], int /*n_types2*/,
	  Tuple *&Jtuple, FldSpec * perm_mat, int nOutFlds)
{
   int   temp_i;
   float temp_f;
   char *temp_s;

   for (int i = 0; i < nOutFlds; i++)
   {
      switch (perm_mat[i].relation)
      {
       case outer:	// Field of outer (t1)
	 switch (type1[perm_mat[i].offset-1])
	 {
	  case attrInteger:
	    Jtuple->setFld(i+1, t1->getFld(perm_mat[i].offset, temp_i));
	    break;
	  case attrReal:
	    Jtuple->setFld(i+1, t1->getFld(perm_mat[i].offset, temp_f));
	    break;
	  case attrString:
	    Jtuple->setFld(i+1, t1->getFld(perm_mat[i].offset, temp_s));
	    break;
	  default:
	    std::cout << "Don't know how to handle attrSymbol, attrNull - projection.C" << std::endl;
	    break;
	 }
	 break;

       case innerRel:	// Field of inner (t2)
	 switch (type2[perm_mat[i].offset-1])
	 {
	  case attrInteger:
	    Jtuple->setFld(i+1, t2->getFld(perm_mat[i].offset, temp_i));
	    break;
	  case attrReal:
	    Jtuple->setFld(i+1, t2->getFld(perm_mat[i].offset, temp_f));
	    break;
	  case attrString:
	    Jtuple->setFld(i+1, t2->getFld(perm_mat[i].offset, temp_s));
	    break;
	  default:
	    std::cout << "Don't know how to handle attrSymbol, attrNull - projection.C" << std::endl;
	    break;
	 }
	 break;
      }
   }
   return;
}




void Project(Tuple * t1, AttrType type1[], int /*n_types1*/,
          Tuple *&Jtuple, FldSpec * perm_mat, int nOutFlds)
{
   int   temp_i;
   float temp_f;
   char *temp_s;

   for (int i = 0; i < nOutFlds; i++)
   {
      switch (perm_mat[i].relation)
      {
       case outer:      // Field of outer (t1)
         switch (type1[perm_mat[i].offset-1])
         {
          case attrInteger:
            Jtuple->setFld(i+1, t1->getFld(perm_mat[i].offset, temp_i));
            break;
          case attrReal:
            Jtuple->setFld(i+1, t1->getFld(perm_mat[i].offset, temp_f));
            break;
          case attrString:
            Jtuple->setFld(i+1, t1->getFld(perm_mat[i].offset, temp_s));
            break;
          default:
            std::cout << "Don't know how to handle attrSymbol, attrNull - projection.C" << std::endl;
            break;
         }
         break;

	default:
	  std::cout <<" something is wrong in perm_mat"<< std::endl;
	  assert(0);
	  break;
      }
    }
    return;
}
