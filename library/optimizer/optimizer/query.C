//
// query.c
//

/*
 * $Id: query.C,v 1.5 1996/05/23 03:51:50 luke Exp $
 * 
 * $Log: query.C,v $
 * Revision 1.5  1996/05/23 03:51:50  luke
 * New heapfiles; rational approach to catalog use by optimizer
 *
 * Revision 1.4  1996/05/01 04:57:23  krisna
 * Donko's changes for better multi-column index print functions.
 *
 * Revision 1.17  1995/11/10 22:30:13  michaell
 * *** empty log message ***
 *
# Revision 1.16  1995/09/01  21:15:37  michaell
# checkin before front in changes.  Index Only now works ok,
# as well as all aggregate like stuff except having
#
# Revision 1.15  1995/08/20  20:03:27  michaell
# sortmerge join finished, working on attributes
# (aggregated attributes)
#
# Revision 1.14  1995/08/04  21:32:44  michaell
# nested loops (of all types but block) work.
#
# Revision 1.13  1995/07/18  00:32:32  michaell
# added sorting, more descriptive output
#
# Revision 1.12  1995/06/17  19:47:30  michaell
# added list instantation, a number of additions to print routines, incld
# descriptions of access methods, and a whole mess of things to more
# accurately calculate the selectivity.
#
# Revision 1.10  1995/05/04  16:13:46  michaell
# minor changes to simplify a few things and to supported ordered relatiosn
#
# Revision 1.9  1995/04/25  21:36:25  michaell
# new selection method works; added a new calculate cost function
#
# Revision 1.8  1995/04/25  18:50:20  michaell
# selects now choose an index appropriately
#
# Revision 1.7  1995/04/24  18:51:35  michaell
# added support to a "range" type of SelectElem, and a number of supporting
# functions and changes to selectivity
#
# Revision 1.6  1995/04/22  21:34:08  michaell
# *** empty log message ***
#
# Revision 1.5  1995/04/20  18:54:36  michaell
# added an OriginalName field to Plan_Relation
#
# Revision 1.4  1995/04/20  16:50:22  michaell
# fixed cardinality problem
#
# Revision 1.3  1995/04/08  23:55:39  michaell
# corrected minor typo
#
# Revision 1.3  1995/04/08  23:55:39  michaell
# corrected minor typo
#
# Revision 1.2  1995/04/08  23:51:49  michaell
# added an attribute id field to the attribute class
#
 *
 */

#include <math.h>
#include <assert.h>
#include "query.h"
#include "StringUtils.h"
#include "opt_globals.h"

AttrValue::AttrValue(): type(attrString), s() { }

AttrValue::AttrValue(const AttrValue & av): s(av.s)
{
  type = av.type;
  if (type == attrInteger)
    i = av.i;
  else if (type == attrReal)
    r = av.r;
}


std::string AttrValue::operator =(const std::string & sVal)
{
  type = attrString;
  s = sVal;
  return (s);
}

long AttrValue::operator =(long iVal)
{
  type = attrInteger;
  return (i = iVal);
}

double AttrValue::operator =(double rVal)
{
  type = attrReal;
  return (r = rVal);
}

AttrValue AttrValue::operator =(const AttrValue & av)
{
  switch (av.type) {
    case attrString: *this = av.s; break;
    case attrInteger: *this = av.i; break;
    case attrReal: *this = av.r; break;
    default: type = av.type;
    }
  return *this;
}

bool AttrValue::operator ==(const AttrValue & av)
{
  if (type == av.type)
    switch (type) {
      case attrString: return (s == av.s);
      case attrInteger: return (i == av.i);
      case attrReal: return (r == av.r);
      default: return -1;
    }
  else
    return -1;
}

bool AttrValue::operator <=(const AttrValue & av)
{
  if (type == av.type)
    switch (type) {
      case attrString: return (s <= av.s);
      case attrInteger: return (i <= av.i);
      case attrReal: return (r <= av.r);
      default: return -1;
    }
  else
    return -1;
}

bool AttrValue::operator >=(const AttrValue & av)
{
  if (type == av.type)
    switch (type) {
      case attrString: return (s >= av.s);
      case attrInteger: return (i >= av.i);
      case attrReal: return (r >= av.r);
      default: return -1;
    }
  else
    return -1;
}

int AttrValue::operator=(int iVal)
{
    type = attrInteger;
    return (int) ( *this = (long) iVal );
}

std::string AttrValue::GetString()
{ 
    return s;
} 

long AttrValue::GetInteger()
{
    return i; 
} 

double AttrValue::GetReal() 
{ 
    return r;
}  

std::ostream & operator << (std::ostream & s, const AttrValue & av)
{
  switch (av.Type()) {
    case attrString: s << av.s; break;
    case attrInteger: s << av.i; break;
    case attrReal: s << av.r; break;
    default: s << "Null"; break;
  }
  return s;
}


std::string AttrValue::MakeString()
{
  std::string result;
  switch (type) {
    case attrString: 
      result = "\"";
      result += s + "\"";
      return result;
    case attrInteger: return LongToString(i);
    case attrReal: return DoubleToString(r);
    default: return "";
  }
}




void AttrValue::Read(std::istream & s, AttrType type)
{
  std::string st;
 
  switch (type) {
    case attrString:
      s >> st; *this = st; break;
    case attrInteger:
      long i; s >> i; *this = i; break;
    case attrReal:
      double r; s >> r; *this = r; break;
    default: break;
  }
}

Index::Index(int iClustered, 
	     int iDistinctKeys,
	     int iNumPages, 
	     SelectType iSelectType, 
	     const AttributeList& iAttributes,
	     std::string iFilename) :
	     filename(iFilename),
	     attributes(iAttributes)
{ 
    if (iClustered)
	clustered = TRUE;
    else
	clustered = FALSE;
    
    attributes.GoHead();
    if (attributes.CurrPtr())
	relation = attributes.CurrPtr()->RelationPtr();
    distinctKeys = iDistinctKeys;
    numPages = iNumPages; 
    selectType = iSelectType; 
} 

  
Index::Index(const Index & i) :
    attributes(i.attributes)
{
    filename = i.filename;
    
    relation = i.relation;  
    clustered = i.clustered;
    distinctKeys = i.distinctKeys;
    numPages = i.numPages; 
    selectType = i.selectType; 
}

bool Index::CanUse(Select* sel)
{
    if (!sel
	|| !(sel->GetExpr()) 
	|| !(sel->GetExpr()->IsElement())
	|| (sel->GetType() == selUndefined)
	|| (GetSelectType() == selExact
	    && sel->GetType() != selExact)) 
	return FALSE;

    for(attributes.GoHead();
	attributes.CurrPtr();
	attributes.GoNext())
    {
	if ( sel->Contains(attributes.CurrPtr()))
	    return TRUE;
    }
    
    return FALSE;    
}

struct PredicateIterator{
     SelectList sl;
     Attribute* att;
     PredicateIterator* next;
     SelectType sel_type;

     bool notIn(Select* select){
		if(select->Contains(att)){
			return false;
		}
		else if(next){
			return next->notIn(select);
		}
		else{
			return true;
		}
	}

     void reset(){
          sl.GoHead();
          Select* sel;
          while((sel = sl.CurrPtr())){
               if(!(sel->GetExpr()) ||
                  !(sel->GetExpr()->IsElement()) ||
                  (sel->GetType() == selUndefined) ||
                  (sel_type == selExact && sel->GetType() != selExact)){
               }
               else if(sel->Contains(att)){

				// We have to verify that sel contains only this attribute

                    if(!next || (next && next->notIn(sel))){
					break;
				}
               }
               sl.GoNext();
          }
          if(next){
               next->reset();
          }
     }

     PredicateIterator(
          const SelectList* sel_l, /* const */ AttributeList& att_l, SelectType sel_t)
          : sel_type(sel_t){
		sl = sel_l;
          att_l.GoHead();
          if(att_l.Size() > 1){
               AttributeList tmp(&att_l);
               tmp.GoHead();
               att = tmp.Detach();
               next = new PredicateIterator(&sl, tmp, sel_t);
          }
          else{
               att = att_l.CurrPtr();
               next = NULL;
          }
     }

     ~PredicateIterator(){
          if(next){
               delete next;
          }
     }

     bool tick(){
          if(!sl.CurrPtr()){
               return false;
          }
          if(next){
               if(next->tick()){
                    return true;
               }
               else{
                    next->reset();
               }
          }
          sl.GoNext();
          Select* sel;
          while((sel = sl.CurrPtr())){
               if(!(sel->GetExpr()) ||
                  !(sel->GetExpr()->IsElement()) ||
                  (sel->GetType() == selUndefined) ||
                  (sel_type == selExact && sel->GetType() != selExact)){

                  // Have to add here the ordered case too!!!
               }
               else if(sel->Contains(att)){

				// We have to verify that sel contains only this attribute

                    if(!next || (next && next->notIn(sel))){
					return true;
				}
               }
               sl.GoNext();
          }
          return false;
     }

     double selectivity(){
          double thisSel = 1;
          if(!sl.CurrPtr()){
               return thisSel;
          }
          thisSel = sl.CurrPtr()->GetSelectivity();
          if(next){
               return thisSel * next->selectivity();
          }
          return thisSel;
     }

     SelectList* list(){
          SelectList* ret_val = new SelectList(TRUE);
		assert(ret_val);
		assert(ret_val->IsEmpty());
          if(sl.CurrPtr() && next){
               *ret_val = next->list();
          }
          if(sl.CurrPtr()){
               ret_val->InsertHead(sl.CurrPtr());
          }
          return ret_val;
     }

     bool complete(){
          if(!sl.CurrPtr()){
               return false;
          }
          else if(next){
               return next->complete();
          }
          else{
               return true;
          }
     }
};

double Index::MostSelective(const SelectList* in_sl, SelectList& out_sl){
     double minSel = 1;
     PredicateIterator iter(in_sl, attributes, selectType);

     double currSel;
     iter.reset();
     if(selectType == selExact && !iter.complete()){
          return minSel;
     }
     do{
          if(selectType == selExact && !iter.complete()){
               continue;
          }
          if((currSel = iter.selectivity()) < minSel){
               minSel = currSel;
               out_sl = iter.list();
          }
     } while(iter.tick());

     return minSel;
}

std::string Index::Filename()
{
    return filename;
}

Plan_Relation* Index::Relation()
{
    return relation;
}

AttributeList& Index::Attributes()
{
    return attributes;
}

bool Index::IsClustered()
{
    return clustered;    
}

int Index::DistinctKeys()
{
    return distinctKeys;
}

int Index::NumPages()
{
    return numPages;
}

SelectType Index::GetSelectType()
{
    return selectType;
}

Attribute::Attribute(const std::string & iName, Plan_Relation *iRelation, 
		     AttrType iType, int iSize, 
		     int iOffset, int iAttrId) : name(iName)
/* , 		     additionalProperties(NULL) */
{
    attrId = iAttrId;
    relation = iRelation; 
    type = iType;
    size = iSize; 
    offset = iOffset; 
    aggregate = NONE;
    aggDistinct = false;
}


Attribute::Attribute(const Attribute & a)
: name(a.name),
  minVal(a.minVal),
  maxVal(a.maxVal)
/* , additionalProperties(a.additionalProperties)  */
{
    attrId = a.attrId;
    relation = a.relation;
    type = a.type;
    size = a.size;
    offset = a.offset;
    aggregate = a.aggregate;   
    aggDistinct = a.aggDistinct;
}

bool Attribute::AggDistinct()
{
    return aggDistinct;
}

Attribute::AggregateType Attribute::MakeAgg( Attribute::AggregateType type,
					     bool iaggDistinct )
{ 
    aggDistinct = iaggDistinct; 
    return (aggregate = type);
} 

Attribute::AggregateType Attribute::GetAgg()
{
    return aggregate;
}


std::string Attribute::FullName()
{
    std::string aggstr;
    std::string disstr;

    if (!aggDistinct && aggregate == COUNT)
    {
	aggstr = "COUNT (*)";
    }
    else 
    {
	if (aggDistinct)
	    disstr = " DISTINCT ";
	else
	    disstr = "";   
	switch (aggregate) 
	{
	  case AVG:
	    aggstr = "AVG ( " + disstr + ((relation->Name() + '.') + name) + ")";
	    break;
	    
	  case MAX:
	    aggstr = "MAX ( " + disstr + ((relation->Name() + '.') + name) + ")";
	    break;
	    
	  case MIN:
	    aggstr = "MIN ( " + disstr + ((relation->Name() + '.') + name) + ")";
	    break;
	    
	  case SUM:
	    aggstr = "MAX ( " + disstr + ((relation->Name() + '.') + name) + ")";
	    break;
	    
	  case COUNT:
	    aggstr = "COUNT ( " + disstr + ((relation->Name() + '.') + name) + ")";
	    break;
	    
	  default:
	    aggstr = ((relation->Name() + '.') + name);
	    break;
	}
    }
    return aggstr;
}


Index *Attribute::FindIndex()
{
    bool done;
    Plan_AccessMethodList aml(&relation->accessMethodList);
    
    for (aml.GoHead(), done = false; !done && aml.CurrPtr(); ) 
    {

	if ( aml.CurrPtr()->IndexPtr() == NULL)
	{
	    aml.GoNext();
	}
	else
	{
	    AttributeList &atts = aml.CurrPtr()->IndexPtr()->Attributes();
	    
	    for (atts.GoHead(); 
	//	 atts.CurrPtr() && (*(atts.CurrPtr())) != (*this); 
		 atts.CurrPtr() && (atts.CurrPtr()->Name() != name);
		 atts.GoNext() );
	    
	    if ( atts.CurrPtr() )
		done = true;
	    else
		aml.GoNext();  
      	    
	}
	
	/*
	   if (!(done = (aml.CurrPtr()->IndexPtr() != NULL)))
	   aml.GoNext();
	    */
    }
    
    
    if (done) return aml.CurrPtr()->IndexPtr();
    else return FALSE;
}

bool Attribute::operator == (const Attribute & a)
{
    return (name == a.name
	    && ((*relation) == (*a.relation))
/*	    && aggregate == a.aggregate
	    && aggDistinct == a.aggDistinct */
	    );
}

std::string Attribute::Name()
{ 
    return name;
}

int Attribute::AttrID()
{
    return attrId; 
}	

Plan_Relation *Attribute::RelationPtr()
{
    return relation;
}

void Attribute::SetRelationPtr(Plan_Relation *iRelation) 
{
    relation = iRelation; 
}

AttrType Attribute::Type()
{
    return type;
}

int Attribute::Size()
{
    return size;    
}

int Attribute::Offset()
{
    return offset;
}

bool Attribute::operator!=(const Attribute & a)
{ 
    return (name != a.name || ((*relation) != (*a.relation))
	    /*
	    || aggregate != a.aggregate || aggDistinct != aggDistinct
              */
	    );
}



Plan_AccessMethod::Plan_AccessMethod()
{
  nodeID = 0;
  cost = 0;
  numLeafPages = 0;  
  tupleOrder = Random;
  attribute = NULL;
  index = NULL;
}

  
Plan_AccessMethod::Plan_AccessMethod(const std::string & iName, 
				     const int & iNumLeafPages, 
				     TupleOrder iTupleOrder, 
				     Attribute *iAttribute,
				     const Index *iIndex): name(iName)
{
    nodeID = nextNodeID++;
    
    numLeafPages = iNumLeafPages;    
    cost = iNumLeafPages; // default cost
    tupleOrder = iTupleOrder;
    attribute = iAttribute;
    if (iIndex) {
	if (!(index = new Index(*iIndex))) FatalError(MEM);
    }
    else index = NULL;
}


Plan_AccessMethod::Plan_AccessMethod(const Plan_AccessMethod & am): name(am.name)
{
    nodeID = nextNodeID++;
    cost = am.cost;
    numLeafPages = am.numLeafPages;
    
    tupleOrder = am.tupleOrder;
    attribute = am.attribute;
    if (am.index) {
	if (!(index = new Index(*am.index))) FatalError(MEM);
    }
    else index = NULL;
} 

Plan_AccessMethod::~Plan_AccessMethod()
{
    if (index)
	delete index;
}

void Plan_AccessMethod::SetCost(const double & iCost)
{
    cost = iCost;
}

int Plan_AccessMethod::NodeID() 
{
    return nodeID;
}

std::string Plan_AccessMethod::Name()
{ 
    return name;
}

double Plan_AccessMethod::Cost()
{
    return cost;
}    

TupleOrder Plan_AccessMethod::Order()
{
    return tupleOrder;
}

Attribute *Plan_AccessMethod::AttributePtr()
{
    return attribute; 
};

void Plan_AccessMethod::SetAttributePtr(Attribute *iAttribute)
{
    attribute = iAttribute; 
}

Index *Plan_AccessMethod::IndexPtr()
{ 
    return index; 
}

void Plan_AccessMethod::SetOrder(TupleOrder iTupleOrder) 
{ 
    tupleOrder = iTupleOrder; 
}


bool Plan_AccessMethod::operator==(const Plan_AccessMethod &am)
{
    return (name == am.name && attribute == am.attribute);
}


SelectType Plan_AccessMethod::GetSelectType()
{
  if (index)
    return index->GetSelectType();
  else if (tupleOrder == Random)
    return selUndefined;
  else
    return selRange;
}


void Plan_AccessMethod::operator=(const Plan_AccessMethod & am)
{
  nodeID = am.nodeID;
  name = am.name;
  numLeafPages = am.numLeafPages;
  cost = am.cost;
  tupleOrder = am.tupleOrder;
  attribute = am.attribute;
  if (index) delete index;  
  if (am.index) 
    if (!(index = new Index(*am.index))) FatalError(MEM);
  else index = NULL;  
}


std::string Plan_AccessMethod::FullName()
{
    std::string rt;
    if (index && index->IsClustered())
	rt = "Clustered ";
    else if ( index )
	rt = "Unclustered ";
    
    rt += name;
    if(index){
	  rt += " on: ";
       AttributeList al(index->Attributes());
       Attribute* att;
       al.GoHead();
       while((att = al.CurrPtr())){
            rt += att->Name();
            al.GoNext();
            if(al.CurrPtr()){
                 rt += ", ";
            }
            else{
                 rt += ".";
                 break;
            }
       }
    }
    else if(tupleOrder != Random){
          rt += " on: " + attribute->Name();
    }

    return rt;
    
}


void Plan_AccessMethod::Show()
{
  std::cout << nodeID << '\n';

  std::cout << "Relation: " << attribute->RelationPtr()->Name();
  std::cout << "\nAccess Method: ";
  
  if (index && index->IsClustered())
      std::cout << "Clustered ";
  else if ( index )
      std::cout << "Unclustered ";
      
  std::cout << name;

  if(index){
       std::cout << " on: ";
       AttributeList al(index->Attributes());
       Attribute* att;
       al.GoHead();
       while((att = al.CurrPtr())){
            std::cout << att->Name();
            al.GoNext();
            if(al.CurrPtr()){
                 std::cout << ", ";
            }
            else{
                 std::cout << ".";
                 break;
            }
       }
  }
  else if (tupleOrder != Random)
      std::cout << " on " << attribute->Name();

  // std::cout << "\nCost: " << cost;

  std::cout << "\nNPages =  "
       << attribute->RelationPtr()->NumPages();

  if (index)
  {
      // # of Index Leaf Pages irrelevent in Hash index
      // (not true with index-only scans)
      if ( index->GetSelectType() == selExact)
      {
	  std::cout << "\nICost = " << index->NumPages();
      }
      else
      {
	  std::cout << "\nIHeight = " << index->NumPages();
      }
      std::cout << "\nINPages = " << numLeafPages;
  }
  
  std::cout << "\nNTuples = "
       << (attribute->RelationPtr()->Cardinality());

  if (index)
  { 
      std::cout << "\nNumber of Distinct Keys = NKeys = "
	   << index->DistinctKeys();
  }
  
  std::cout << "[";

  /*
  std::cout << "\n\nCost Description:" << endl;
    
  if ( (!index && (tupleOrder == Random ))
      || (index && index->GetSelectType() == selUndefined ))
  {
      std::cout << "  The cost to use this access method\n"
	  << "   is the number of pages in the relation." << endl;
  }
  else if ( !index ) // sorted by this column, and worth using.
  {
      double numPages = attribute->RelationPtr()->NumPages();
      double costToGetThere = 1;
      if (numPages) 
	    costToGetThere = log(numPages)/ log(2);

      std::cout << "  The cost to use this access method\n"
	   << "  is the log of the number of pages in\n"
	   << "  the relation ( " << costToGetThere << " )\n"
	   << "  plus the number of pages returned.\n"
           << "  The number of pages returned is estimated\n" 
           << "  based on the selectivity of the primary condition.\n"
           << "  Minirel currently does not implement this method."
	   << endl;
  }    
  else if (index->GetSelectType() == selExact && index->IsClustered())
  {
      std::cout << "  The cost to use this access method\n"
           << "  is the number of pages returned, plus\n"
	   << "  the number of pages to reach the leaf bucket.\n"
           << "  The number of pages returned is estimated\n" 
           << "  based on the selectivity of the primary condition.\n"
           << "  Minirel currently does not implement this method."
	   << endl;
  }
  else if (index->GetSelectType() == selExact) // not clustered
  {
      std::cout << "  The cost to use this access method\n"
	   << "  is the number of tuples returned plus\n"
           << "  the number of pages to reach the leaf bucket.\n"
           << "  The number of tuples returned is estimated\n" 
           << "  based on the selectivity of the primary condition."
	   << endl;
  }
  else if (index->IsClustered()) // selRange or selBoth, a B+ Index.
  {
      std::cout << "  The cost to use this access method\n"
	   << "  is the number of pages returned plus\n"
	   << "  the cost to reach the leaf pages plus\n"
           << "  the number of leaf pages that are read.\n"
	   << "  The number of pages and leaf pages are\n"
           << "  estimated based on the selectivity of the\n"
	   << "  primary condition.\n"
           << "  Minirel currently does not implement this method."
           << endl;
    }
    else // index isn't clustered
    {
	std::cout << "  The cost to use this access method\n"
	     << "  is the number of tuples returned plus\n"
	     << "  the cost to reach the leaf pages plus\n"
             << "  the number of leaf pages that are read.\n"
	     << "  The number of tuples and leaf pages are\n"
             << "  estimated based on the selectivity of the\n"
	     << "  primary condition."
	     << endl;
    }
    */


  AttributeList *al;
  std::cout << "\nAttributes:";
  al = &attribute->RelationPtr()->attributeList;
  for (al->GoHead(); al->CurrPtr(); al->GoNext()) {
      std::cout << "\n>   " + al->CurrPtr()->FullName(); //RelationPtr()->Name();
      // std::cout << "." << al->CurrPtr()->Name();
      if (al->CurrPtr() == attribute 
	  && tupleOrder != Random)
	  switch (tupleOrder) {
	    case Ascending: std::cout << "  (Ascending)"; break;
	    case Descending: std::cout << "  (Descending)"; break;
	    default: break;
	  }
      std::cout << " (Min = " << al->CurrPtr()->minVal 
           << ", Max = " << al->CurrPtr()->maxVal
	   << " )";
      
  }
  
  std::cout << "]\n";
  std::cout << ".:\n";

}

std::string Plan_AccessMethod::GetDescription(bool indexOnly)
{
    std::string temp;
    
    if ( (!index && (tupleOrder == Random ))
	|| (index && index->GetSelectType() == selUndefined ))
	temp = " NPages ";
    else if ( !index )
	temp = " log(NPages) + (NPages * RF) ";
    else if (index->GetSelectType() == selExact && index->IsClustered() 
	     && indexOnly)
	temp = " INPages ";
    else if (index->GetSelectType() == selExact && index->IsClustered())
	temp = " (NPages * RF) + ICost ";
    else if (index->GetSelectType() == selExact && indexOnly) // not clustered
	temp = " INPages ";
    else if (index->GetSelectType() == selExact) // not clustered
	temp = " NPages(Fetched) + ICost ";
    else if (index->IsClustered() && indexOnly) // a B+ Index, index only
	temp = " IHeight + (INPages * RF) ";
    else if (index->IsClustered()) // selRange or selBoth, a B+ Index.
	temp = " IHeight + (INPages * RF) + (NPages * RF)";
    else if (indexOnly) 
	temp = " IHeight + (INPages * RF)";
    else 
	temp = " IHeight + (INPages * RF) + NPages(Fetched) ";
    
    return temp;   
}

double Plan_AccessMethod::CalculateCost(double selectivity, bool indexOnly)
{
    double cardinality = attribute->RelationPtr()->Cardinality();
    
    if ( (!index && (tupleOrder == Random || selectivity == 1))
	|| (index && index->GetSelectType() == selUndefined ))
	return attribute->RelationPtr()->NumPages();
    else if ( !index ) // sorted by this column, and worth using.
    {
	double numPages = attribute->RelationPtr()->NumPages();
	double CostToGetThere = 1;
	if (numPages) 
	    CostToGetThere = log(numPages)/ log(2);

	double NumPagesReturned =(numPages * selectivity);
	return NumPagesReturned + CostToGetThere;
    }    
    else if (index->GetSelectType() == selExact && index->IsClustered())
    {
	// cost 
	// double numRecReturn = (cardinality * selectivity);
	// double pagePerRec = attribute->RelationPtr()->NumPages() / cardinality;
	// double numPageReturn = (numRecReturn * pagePerRec);
	double numPageReturn = attribute->RelationPtr()->NumPages() * selectivity;
	
	if (numPageReturn < 1) numPageReturn = 1;

	// clustered hash index.
	if (indexOnly)
	    return numLeafPages;
		// index->NumPages();
	else
	    return numPageReturn + index->NumPages();
    }
    else if (index->GetSelectType() == selExact) // not clustered
    {
	// cost is # of tuples returned + NumPages in index
	double numRecReturn = (cardinality * selectivity);
	if (indexOnly)
	    return numLeafPages;
		// index->NumPages();
	else
	    return numRecReturn + index->NumPages();
    }
    else if (index->IsClustered()) // selRange or selBoth, a B+ Index.
    {
	// double numRecReturn = (cardinality * selectivity);
	// double pagePerRec = attribute->RelationPtr()->NumPages() / cardinality;
	// double numPageReturn = (numRecReturn * pagePerRec);
	double numPageReturn = attribute->RelationPtr()->NumPages() * selectivity;
	double leafPages = (selectivity * numLeafPages);
	if (numPageReturn < 1) numPageReturn = 1;
	if (leafPages < 1) leafPages = 1;
	if (indexOnly)
	    return leafPages + index->NumPages();
	else
	    return numPageReturn + leafPages + index->NumPages();
    }
    else // index isn't clustered
    {
	double numRecReturn = (cardinality * selectivity);
	if (numRecReturn < 1) numRecReturn = 1;	
	double leafPages = (selectivity * numLeafPages);
	if (leafPages < 1) leafPages = 1;
	if (indexOnly)
	    return leafPages + index->NumPages();
	else
	    return numRecReturn + leafPages + index->NumPages();
    }
}



Plan_Relation::Plan_Relation(): name(), originalName(), accessMethodList(),
                                attributeList(),
			        additionalPropertyList() { }

Plan_Relation::Plan_Relation(const std::string & iName, DataBase *iDb, ulong iCardinality,
		int iTupleSize, int iNumPages/*,Relation *rel  (SCH)*/): 
		name(iName), originalName(iName), accessMethodList(),
		attributeList(), additionalPropertyList() //, relation(rel)  (SCH)
{
  db = iDb;
  cardinality = iCardinality;
  tupleSize = iTupleSize;
  numPages = iNumPages;
}

Plan_Relation::Plan_Relation(const Plan_Relation & r): name(r.name),
                        originalName(r.originalName),
			accessMethodList(r.accessMethodList),
			attributeList(r.attributeList),
			additionalPropertyList(r.additionalPropertyList)
                     // (SCH), relation(r.relation)
{
  for (accessMethodList.GoHead(); accessMethodList.CurrPtr(); 
       accessMethodList.GoNext()) {
    attributeList.Goto(*accessMethodList.CurrPtr()->AttributePtr());
    accessMethodList.CurrPtr()->SetAttributePtr(attributeList.CurrPtr());
  }
  
  for (attributeList.GoHead(); attributeList.CurrPtr(); attributeList.GoNext())
    attributeList.CurrPtr()->SetRelationPtr(this);

  db = r.db;
  cardinality = r.cardinality;
  tupleSize = r.tupleSize;
  numPages = r.numPages;
}


void SelectOperand::Show()
{
  if (literal) std::cout << av;
  else std::cout << a->FullName();
      // std::cout << a->RelationPtr()->Name() << "." << a->Name();
}


std::string SelectOperand::MakeString()
{
  std::string result;

  if (literal) result += av.MakeString();
  else {
      result += a->FullName();
//    result += a->RelationPtr()->Name();
//    result += ".";
//    result += a->Name();
  }

  return result;
}

int SelectOperand::operator==(const SelectOperand & so)
{
    if (literal == so.literal)
    {
	if (literal)
	    return av == so.av;
	else
	    return a== so.a;
    }
    else
	return 0;
}


SelectElem::SelectElem(const SelectElem & se)
{
  op = se.op;
  op2 = se.op2;
  
  if (se.arg1) arg1 = new SelectOperand(*se.arg1);
  else arg1 = NULL;
  if (se.arg2) arg2 = new SelectOperand(*se.arg2); 
  else arg2 = NULL;
  if (se.arg3) arg3 = new SelectOperand(*se.arg3);
  else arg3 = NULL;
  
}

SelectElem::SelectElem(AttrOperator iOp, SelectOperand *iArg1, SelectOperand *iArg2)
{
    arg1 = arg2 = arg3 = NULL;
    op = op2 = OPTRUE;

    if ( ! iArg1->IsLiteral() )
    {
	arg2 = iArg1; // put the column in the middle
	switch ( iOp)
	{
	  case aopEQ:
	    op = SelectElem::EQUAL; arg1 = iArg2; break;
	  case aopNE:	    
	    op = SelectElem::NOTEQUAL ; arg1 = iArg2; break;
	  case aopLT:
	    op2 = SelectElem::LESSTHAN; arg3 = iArg2; break;
	  case aopLE:
	    op2 = SelectElem::LESSEQUAL; arg3 = iArg2; break;
	  case aopGT:
	    op = SelectElem::LESSTHAN ; arg1 = iArg2; break;
	  case aopGE:
	    op = SelectElem::LESSEQUAL; arg1 = iArg2; break;
	  case aopNOT:
	    op = SelectElem::EQUAL;
	    arg1 = new SelectOperand;
	    *arg1 = 0; break;
	    
	  case aopNOP:
	    op = op2 = SelectElem::OPTRUE;
	    arg1 = new SelectOperand;
	    *arg1 = 0;
	  default:
	    break;
	    
	}		
    }
    else
    {	
        arg2 = iArg2;
	 // put the column in the middle
        switch ( iOp)     {
	  case aopEQ:
	    op = SelectElem::EQUAL; arg1 = iArg1; break;
	  case aopNE: 
	    op = SelectElem::NOTEQUAL; arg1 = iArg1; break;
          case aopLT:
	    op = SelectElem::LESSTHAN; arg1 = iArg1; break;
	  case aopLE:
	    op = SelectElem::LESSEQUAL; arg1 = iArg1; break;
          case aopGT:
	    op = SelectElem::LESSTHAN; arg3 = iArg1; break;
	  case aopGE:
	    op = SelectElem::LESSEQUAL; arg3 = iArg1; break;
	  case aopNOT:
	    op = SelectElem::EQUAL;
	    arg1 = new SelectOperand;
	    *arg1 = 0;
	    break;
	    
	  case aopNOP:
	    op = SelectElem::NOTEQUAL;
	    arg1 = new SelectOperand;
	    *arg1 = 0;
	    break;
	    
	  default:	  	  
	   break; 
        }
	
    }
}

AttrOperator GetOp(SelectElem::Operator op)
{
    switch(op)
    {
      case SelectElem::LESSTHAN:
	return aopLT;
      case SelectElem::LESSEQUAL:
	return aopLE;
      case SelectElem::EQUAL:
	return aopEQ;
      case SelectElem::NOTEQUAL:
	return aopNE;
      default:
	return aopNOP;	
    }
    
}

AttrOperator SelectElem::GetOperator1()
{
    return GetOp(op);
}

AttrOperator SelectElem::GetOperator2()
{
    return GetOp(op2);
}


// the following three functions are a kludge to get it to work
// with the spring 1995 764 project.  eventually select elem will
// support full ranges (it already does, but it has been removed
// because minirel does not.

// note, they assume that it isn't a "full" range query.

AttrOperator SelectElem::OldGetOperator()
{
    if (arg1)
	return GetOperator1();
    else
	return GetOperator2();   
}

SelectOperand *SelectElem::OldArg1()
{
    if (arg1)
	return arg1;
    else
	return arg2;    
}

SelectOperand *SelectElem::OldArg2()
{
    if (arg1)
	return arg2;
    else
	return arg3;
}

double SelectElem::GetSelectivity()
{
    if ( op == OPTRUE && op2 == OPTRUE)
	return 1.0;
    else if ( op == OPFALSE  && op2 == OPFALSE)
	return 0;
    else if ( op == EQUAL || op == NOTEQUAL )
    {
	int distKeys1, distKeys2;
	Index *index;
	// note, no arg3
	if ( arg1 && !(arg1->IsLiteral()))
	{
	    index = arg1->AttributePtr()->FindIndex();
	    if (index) distKeys1 = index->DistinctKeys();
	    else distKeys1 = SelectElem::SELRATIO;	    
	}  else distKeys1 = 1; 
	
	if ( arg2 && !(arg2->IsLiteral()))
	{
	    index = arg2->AttributePtr()->FindIndex();
	    if (index) distKeys2 = index->DistinctKeys();
	    else if ( distKeys1 == 1) distKeys2 = SelectElem::SELRATIO;
	    else distKeys2 = 1;
	} else distKeys2 = 1;

	if (distKeys1 <= 0) distKeys1 = 1;
	if (op == NOTEQUAL)
	    return (1.0 - (1.0 / (double)_MAX(distKeys1, distKeys2)));
	else
	    return (1.0 / (double)_MAX(distKeys1, distKeys2));
    }
    else // op == lessthan || op == lessequal
    {
	double result, min, max, colmin, colmax;
	
	if ( arg2->AttributePtr()->Type() == attrInteger )
	{
	    colmax = arg2->AttributePtr()->maxVal.GetInteger();	    
	    colmin = arg2->AttributePtr()->minVal.GetInteger();
	}
	else
	{
	    colmax = arg2->AttributePtr()->maxVal.GetReal();
	    colmin = arg2->AttributePtr()->minVal.GetReal();
	}
	
	if ( !arg1)
	{
	    if ( arg2->AttributePtr()->Type() == attrInteger )
		min = arg2->AttributePtr()->minVal.GetInteger();
	    else
		min = arg2->AttributePtr()->minVal.GetReal();
	}
	else if ( arg1->IsLiteral() )
	{
	    if ( arg2->AttributePtr()->Type() == attrInteger )
		min = arg1->GetLiteral().GetInteger();
	    else if ( arg2->AttributePtr()->Type() == attrReal )
		min = arg1->GetLiteral().GetReal();	    
	    else // string, return default percentage
		if (arg3) return (.25); else return (.3);
	}
	else // arg1 is also a column
	    if (arg3) return (.25); else return (.3);
	
	if ( !arg3)
	{
	    if ( arg2->AttributePtr()->Type() == attrInteger )
		max = arg2->AttributePtr()->maxVal.GetInteger();
	    else
		max = arg2->AttributePtr()->maxVal.GetReal();
	}
	else if ( arg3->IsLiteral() )
	{
	    if ( arg2->AttributePtr()->Type() == attrInteger )
		max = arg3->GetLiteral().GetInteger();
	    else if ( arg2->AttributePtr()->Type() == attrReal )
		max = arg3->GetLiteral().GetReal();	    
	    else // string, return default percentage
		if (arg1) return (.25); else return (.3);
	}
	else // arg1 is also a column
	    if (arg1) return (.25); else return (.3);
	
	result = ( max - min ) / ( colmax - colmin);
	
        if (result >= 1) return 1;
	else if (result >= 0.0) return result;
	else return 0.0;  
    }
}    

void SelectElem::Show()
{
  if ( !arg1 && !arg2 && !arg3)
  {
      if (op == OPTRUE)
	  std::cout << " TRUE ";
      else
	  std::cout << " FALSE ";
      return;
  }
  
  std::cout << "(";
  if (arg1) arg1->Show();
  
  switch (op) {
    case EQUAL:  std::cout << " = "; break;
    case LESSTHAN:  std::cout << " < "; break;
    case NOTEQUAL:  std::cout << " != "; break;
    case LESSEQUAL:  std::cout << " <= "; break;
    default:  std::cout << " "; break;
  }
  if (arg2) {
    arg2->Show();
  }
  switch (op2) {
    case EQUAL: std::cout << " = "; break;
    case LESSTHAN: std::cout << " < "; break;
    case NOTEQUAL: std::cout << " != "; break;
    case LESSEQUAL: std::cout << " <= "; break;
    default: std::cout << " "; break;
  }
  if (arg3) arg3->Show();
  std::cout << ")";
  
} 


std::string SelectElem::MakeString()
{
  std::string result;

  if ( !arg1 && !arg2 && !arg3)
  {
      if ( op == OPTRUE) 
	  result += " TRUE ";
      else
	  result += " FALSE ";
      
      return result;
  }

  result += "(";
  
  if (arg1) result += arg1->MakeString();
  
  switch (op) {
    case EQUAL:  result += " = "; break;
    case LESSTHAN:  result += " < "; break;
    case NOTEQUAL:  result += " != "; break;
    case LESSEQUAL:  result += " <= "; break;
    default:  break;
  }
  if (arg2) {
    result += arg2->MakeString();
  }
  switch (op2) {
    case EQUAL:  result += " = "; break;
    case LESSTHAN:  result += " < "; break;
    case NOTEQUAL:  result += " != "; break;
    case LESSEQUAL:  result += " <= "; break;
    default:  break;
  }
  
  if (arg3) result+=arg3->MakeString();
  result += ")";
  
  return result;
} 

bool SelectElem::TryToInclude( SelectElem& toInclude)
{
    if ( (*arg2 != *(toInclude.arg2)) // the attributes don't match
	|| (arg1 && !(arg1->IsLiteral())) // our left most isn't a literal
	|| (toInclude.arg1 && !(toInclude.arg1->IsLiteral())) // nor toInclude's
	|| (arg3 && !(arg3->IsLiteral())) // our right most isn't a literal
	|| (toInclude.arg3 && !(toInclude.arg3->IsLiteral()))) // nor toInclude's
	return FALSE; // it isn't any use to us
  
    bool ok = false;

    // now, it probably is.
    if ( toInclude.arg1 ) // something on the left hand side
    {
	if ( !arg1 ) // nothing on the left hand side -- easy.
	{
	    if (toInclude.GetType() == selRange)
	    {
		arg1 = new SelectOperand(*toInclude.arg1);
		op = toInclude.op;
		ok = true;
	    }
	}
	else // both arg1 and toInclude.arg1 exist.  time to arbitrate
	{
	    if ( GetType() == selRange && toInclude.GetType() == selRange)
	    {
		if ( toInclude.arg1->GetLiteral() < arg1->GetLiteral() )
		{
		    ok = true;
		}
		else if ( toInclude.arg1->GetLiteral() == arg1->GetLiteral()
			 && op == LESSTHAN )
		{
		    ok = true;
		}
		else // their's is better
		{
		    *arg1 = *(toInclude.arg1);
		    op = toInclude.op;
		    ok = true;
		}
	    }
	    else if ( ( op == NOTEQUAL && toInclude.op == EQUAL  )
		     || ( op == EQUAL && toInclude.op == NOTEQUAL ))
	    {
		if ( toInclude.arg1->GetLiteral() == arg1->GetLiteral() )
		{
		    // always false!
		    ok = true;
		    delete arg1; delete arg2;
		    arg1 = arg2 = arg3 = NULL;
		    op = OPFALSE;
		}
		else ok = true; // it's irrelevant.
	    }
	    else if ( op == NOTEQUAL && toInclude.op == NOTEQUAL)
		ok=(toInclude.arg1->GetLiteral() == arg1->GetLiteral());
	    else if ( op == EQUAL && toInclude.op == EQUAL)
	    {
		if ( toInclude.arg1->GetLiteral() != arg1->GetLiteral())
		{
                    // always false!
                    ok = true;
                    delete arg1; delete arg2;
                    arg1 = arg2 = arg3 = NULL;
                    op = OPFALSE;
		}
		else ok = true;		 // it's irrelevant.
            }
	}
    }
    
    if (toInclude.arg3)
    {
	if ( !arg3 ) // nothing on the right side -- easy.
	{
            if (toInclude.GetType() == selRange)
            {
                arg3 = new SelectOperand(*(toInclude.arg3));
                op2 = toInclude.op2;
                ok = true;
            }
        }
	else
	{
	    if ( GetType() == selRange && toInclude.GetType() == selRange)
            {		    
		if ( toInclude.arg3->GetLiteral() > arg3->GetLiteral() )
		{
		    ok = true;
		}
		else if ( toInclude.arg3->GetLiteral() == arg3->GetLiteral()
			 && op2 == LESSTHAN )
		{
		    ok = true;
		}
		else // their's is better
		{
		    *arg3 = *(toInclude.arg3);
		    op2 = toInclude.op2;
		    ok = true;
		}
	    } // if ( GetType() == selRange && toInclude.GetType() ==
	} // else
    } // if (toInclude.arg3)
    
    return ok;  
}


SelectTerm::SelectTerm(const SelectTerm & st)
{
  op = st.op;
  if (st.sel1) sel1 = new SelectExpr(*st.sel1);
  else sel1 = NULL;
  if (st.sel2) sel2 = new SelectExpr(*st.sel2);
  else sel2 = NULL;
}


SelectTerm::~SelectTerm()
{
  if (sel1) delete sel1; if (sel2) delete sel2;
}


double SelectTerm::GetSelectivity()
{
  if (!sel1 || ((op == lopAND || op == lopOR) && !sel2)) {
    std::cerr << "Error in SelectTerm::GetSelectivity(), missing select expression\n";
    return 1.0;
  } else
    switch(op) {
      case lopAND:
        return (sel1->GetSelectivity() * sel2->GetSelectivity());
      case lopOR:
	double d1, d2;
	d1 = sel1->GetSelectivity();
	d2 = sel2->GetSelectivity();
	return (d1 + d2 - (d1 * d2));
      case lopNOT:
	return (1.0 - sel1->GetSelectivity());
      default:
	return 1.0;
    }
}


void SelectTerm::Show()
{
   if (sel1 && sel2) {
    std::cout << "(";
    sel1->Show();
  }
  switch (op) {
    case lopAND:  std::cout << " AND "; break;
    case lopOR:  std::cout << " OR "; break;
    case lopNOT:  std::cout << " ! "; break;
    default:  std::cout << " "; break;
  }
  if (sel2) {
    sel2->Show();
    std::cout << ")";
  }
  else
    sel1->Show(); 
}


std::string SelectTerm::MakeString()
{
  std::string result;

  if (sel1 && sel2) {
    result += "(";
    result += sel1->MakeString();
  }
  switch (op) {
    case lopAND:  result += " AND\n"; break;
    case lopOR:  result += " OR\n"; break;
    case lopNOT:  result += " NOT"; break;
    default:  break;
  }
  if (sel2) {
    result += sel2->MakeString();
    result += ")";
  }
  else
    result += sel1->MakeString();

  return result;
}

bool SelectTerm::Contains(Attribute *a)
{
    return ( (sel1 && sel1->Contains(a))
	    || ( sel2 && sel2->Contains(a)));
}


SelectExpr::SelectExpr(const SelectExpr & se)
{
  isElem = se.isElem;
  if (isElem && se.elem) elem = new SelectElem(*se.elem);
  else if (!isElem && se.term) term = new SelectTerm(*se.term);
}


double SelectExpr::GetSelectivity()
{
  if (isElem && elem) return elem->GetSelectivity();
  else if (!isElem && term) return term->GetSelectivity();
  else {
    std::cerr<< "Error in SelectExpr::GetSelectivity(), pointer to NULL term or elem\n";
    return 1.0;
  }
}


void SelectExpr::Show()
{
  if (isElem && elem) elem->Show();
  else if (term) term->Show();
  else std::cerr << "error in SelectExpr::Show();\n";
}


std::string SelectExpr::MakeString()
{
  if (isElem && elem) return elem->MakeString();
  else if (term) return term->MakeString();
  else { std::cerr << "error in SelectExpr::MakeString()\n"; return ""; }
}



Attribute *Select::GetAttribute(SelectOperand * so)
{
  if (so == NULL)
    return NULL;
  else if (so->IsLiteral())
    return NULL;
  else
    return so->AttributePtr();
}


void Select::RGetAttributeList(SelectExpr *expr, AttributeList & al)
{
  Attribute *a;

  if (expr) {
    if (expr->IsElement() && expr->GetElem()) {
      a = GetAttribute(expr->GetElem()->Arg1());
      if (a && !al.Goto(a))
        al.InsertTail(a);
      a = GetAttribute(expr->GetElem()->Arg2());
      if (a && !al.Goto(a))
        al.InsertTail(a);
      a = GetAttribute(expr->GetElem()->Arg3());
      if (a && !al.Goto(a))
	  al.InsertTail(a);
    }
    else if (expr->GetTerm()) {
      RGetAttributeList(expr->GetTerm()->Sel1(), al);
      RGetAttributeList(expr->GetTerm()->Sel2(), al);
    }
  }
}


SelectType Select::GetType()
{
  if (expr)
    if (expr->IsElement() && expr->GetElem())
	return expr->GetElem()->GetType();
  
    return selUndefined;
}


int Select::TypeCompatible(SelectType st)
{
  switch (GetType()) {
    case selRange: return (st == selRange || st == selBoth);
    case selExact: return (st == selExact || st == selBoth);
    default: return FALSE;
    }
}


AttributeList *Select::GetAttributeList()
{
  AttributeList *al = new AttributeList(TRUE);
  if (!al) FatalError(MEM);

  RGetAttributeList(expr, *al);
  return al;
}

int Select::CanDoSelect(const Plan_Relation *r)
{
  AttributeList *al;
  int done;

  al = GetAttributeList();
  for (done = 0, al->GoHead(); !done && al->CurrPtr(); al->GoNext())
    done = (al->CurrPtr()->RelationPtr() != r);
  delete al;
  return !done;
}


int Select::CanDoSelect(const Plan_RelationList & rl)
{
  Plan_RelationList temp(&rl);
  AttributeList *al;
  int done;

  al = GetAttributeList();
  for (done = 0, al->GoHead(); !done && al->CurrPtr(); al->GoNext())
    done = (temp.Goto(al->CurrPtr()->RelationPtr()) == NULL);
  al->UnLink();
  delete al;
  return !done;
}


double Select::GetSelectivity()
{
  if (expr) return expr->GetSelectivity(); else return 1.0;
}

bool Select::Contains(Attribute *a)
{
    if (!expr)
	return FALSE;
    else
	return expr->Contains(a);
}

State::State(const State & state): joins(&state.joins),
				   project(&state.project),
				   neededAttributes(&state.neededAttributes),
				   selects(&state.selects),
				   groupby(&state.groupby),
				   having(&state.having),
                                   omits(&state.omits),
                                   order(&state.order)
{ 
    distinct = state.distinct;
    containsAggregates = state.containsAggregates;
    aggDistinct = state.aggDistinct;  
}



template class List<PropertyNode>;
template class List<Attribute>;
template class List<Plan_AccessMethod>;
template class List<Plan_Relation>;
template class List<Select>;
template class List<OrderAttr>;

template class ListBase<PropertyNode>;
template class ListBase<Attribute>;
template class ListBase<Plan_AccessMethod>;
template class ListBase<Plan_Relation>;
template class ListBase<Select>;
template class ListBase<OrderAttr>;

template class DList<PropertyNode>;
template class DList<Attribute>;
template class DList<Plan_AccessMethod>;
template class DList<Plan_Relation>;
template class DList<Select>;
template class DList<OrderAttr>;

template class ListNode<PropertyNode>;
template class ListNode<Attribute>;
template class ListNode<Plan_AccessMethod>;
template class ListNode<Plan_Relation>;
template class ListNode<Select>;
template class ListNode<OrderAttr>;

