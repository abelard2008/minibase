//
// boguscatalogs.c
//
// C++ Minirel System Catalogs
//
// Adapted by Stephen Harris from boguscatalogs.c written 
// for SHORE Persistent Object System Software.
//

/*
 * $Id: boguscatalogs.C,v 1.5 1996/05/23 03:52:04 luke Exp $
 *
 * $Log: boguscatalogs.C,v $
 * Revision 1.5  1996/05/23 03:52:04  luke
 * New heapfiles; rational approach to catalog use by optimizer
 *
 * Revision 1.4  1996/04/21 15:19:50  luke
 * Revamped space management; moved to gcc v2.7.1
 *
 * Revision 1.3  1996/03/11 15:39:47  luke
 * Purified
 *
 * Revision 1.2  1996/02/15 03:07:12  luke
 * Incorporated Donko's multi-attribute index changes
 *
 */

/**********************************************************************
* SHORE Persistent Object System Software
* Copyright (c) 1991 Computer Sciences Department, University of
*                    Wisconsin -- Madison
* All Rights Reserved.
*
* Permission to use, copy, modify and distribute this software and its
* documentation is hereby granted, provided that both the copyright
* notice and this permission notice appear in all copies of the
* software, derivative works or modified versions, and any portions
* thereof, and that both notices appear in supporting documentation.
*
* THE COMPUTER SCIENCES DEPARTMENT OF THE UNIVERSITY OF WISCONSIN --
* MADISON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION.  
* THE DEPARTMENT DISCLAIMS ANY LIABILITY OF ANY KIND FOR ANY DAMAGES
* WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
*
* The SHORE Project Group requests users of this software to return 
* any improvements or extensions that they make to:
*
*   SHORE Project Group 
*     c/o David J. DeWitt and Michael J. Carey
*   Computer Sciences Department
*   University of Wisconsin -- Madison
*   Madison, WI 53706
*
*	 or shore@cs.wisc.edu
*
* In addition, the SHORE Project Group requests that users grant the 
* Computer Sciences Department rights to redistribute these changes.
**********************************************************************/
#include "boguscatalogs.h"
#include "ioroutines.h"

#include <ostream>
#include <string>
#include <stdlib.h>
#include <ctype.h>

DataBase::DataBase(std::string filenameArg)
    :numberofrelations(0), relations(), filename(filenameArg)
{

  std::ifstream infile (filename, std::ios::in);

  if (!infile)
    {
#ifdef DEBUG
      std::cerr << "error opening file.: " << filename << std::endl;
#endif
      return;
    }

  infile >> numberofrelations;

  for (int i=0; i<numberofrelations; i++) 
    relations.InsertTail(ReadPlan_Relation(infile));
}

Plan_Relation *DataBase::ReadPlan_Relation (std::ifstream &infile)
{
  std::string name;
    int numAttrs;
    ulong cardinality;
    int tupleSize;
    int numPages;
    
    
    infile >> std::ws;
    infile >> name;
    
    infile >> numAttrs; // number of attributes
    infile >> cardinality;
    infile >> tupleSize;
    infile >> numPages;
    
    Plan_Relation *result = new Plan_Relation(name, this, cardinality, tupleSize, numPages);
    
    
    for (int i=0; i < numAttrs; i++)
    {
	Attribute *a = ReadAttribute(infile, result);
	
	result->attributeList.InsertTail(a);
	
	int amNum;
	infile >> amNum;   // read number of access methods
	for (int j = 0; j < amNum; j++)
	{
	    Plan_AccessMethod *am = ReadPlan_AccessMethod(infile, a);
	    if ( am && !(am->IndexPtr()) && am->Order() != Random)
	    {
		// it's an ordered list, which gives us two options
		result->accessMethodList.InsertTail(am);
		// the ordered one
		
		am = new Plan_AccessMethod(*am);
		am->SetOrder(Random);
		// and an unordered one
	    }
	    
	    result->accessMethodList.InsertTail(am);
	}
    }

  // Fixed the empty Attributes entered by multikey AM here

  Plan_AccessMethod *am;
  result->accessMethodList.GoHead();
  while((am = result->accessMethodList.CurrPtr())){
       if(am->IndexPtr()){
            Attribute* currAtt;
            am->IndexPtr()->Attributes().GoHead();
            while((currAtt = am->IndexPtr()->Attributes().CurrPtr())){
                 if(currAtt->RelationPtr() == NULL){
                      Attribute* relAtt;
                      result->attributeList.GoHead();
                      while((relAtt = result->attributeList.CurrPtr())){
                           if(currAtt->Name() == relAtt->Name()){
                                *currAtt = *relAtt;
                                break;
                           }
                           result->attributeList.GoNext();
                      }
                 }
                 if(currAtt->RelationPtr() == NULL){
                      std::cerr << "Attribute " << currAtt->Name()
                           << " is not member of relation " << name << std::endl;
                      exit(1);
                 }
                 am->IndexPtr()->Attributes().GoNext();
            }
       }
       result->accessMethodList.GoNext();
  }

    return result;
    
}

Attribute *DataBase::ReadAttribute (std::ifstream &infile, Plan_Relation *relation)
{
  std::string name;
  AttrType type;
  int size, offset;
  AttrValue minVal, maxVal;
  int attrID;

  infile >> std::ws;
  infile >> name;
  infile >> attrID;
  infile >> type;
  minVal.Read(infile, type);
  maxVal.Read(infile, type);
  infile >> size;
  infile >> offset;// wonder why manuvir wants this...

  Attribute *result = new Attribute(name, relation, type, size, offset, attrID);
  result->minVal = minVal;
  result->maxVal = maxVal;

  return result;
}



Plan_AccessMethod *DataBase::ReadPlan_AccessMethod (std::ifstream &infile, Attribute *attribute)
{

  std::string name;
  int numLeafPages;
  TupleOrder tupleOrder;
  Index *index = NULL;
 
  infile >> std::ws;
  infile >> name;
  infile >> numLeafPages;
  infile >> tupleOrder;
  int isIndexed;
  infile >> isIndexed;
  if (isIndexed) {
    int clustered, distinctKeys, numOverHeadPages, selType;
    SelectType selectType;
    std::string filename;
    
    infile >> clustered;
    infile >> distinctKeys;
    infile >> numOverHeadPages;
    infile >> selType;
    switch (selType) {
      case 0: selectType = selRange; break;
      case 1: selectType = selExact; break;
      case 2: selectType = selBoth; break;
      default: selectType = selUndefined; break;
      }
    infile >> filename;
    
    AttributeList al;
    al.InsertTail(*attribute);
    
// This part reads the additional keys that this index might contain.
// Adittional attributes must be listed between percentage signs.
// For example: % key2 key3 % denotes two additional keys

  char c;
  while(infile.get(c) && isspace(c)){
  }
  if(c != '%'){
       infile.putback(c);
  }
  else{

       // we have multiattribute index

    std::string attName;

       while(infile.get(c)){
            if(isspace(c)){
                 continue;
            }
            if(c != '%'){
                 infile.putback(c);
                 infile >> attName;
                 Attribute* att = new Attribute(attName);
                 al.InsertTail(att);
            }
            else{
                 break;
            }
       }
  }

    if (!(index = new Index(clustered, distinctKeys, 
			    numOverHeadPages, selectType, 
			    al, filename))) FatalError(MEM);
  }

  Plan_AccessMethod* am;
  am = new Plan_AccessMethod(name, numLeafPages, tupleOrder, attribute, index);
  delete index; // Index copies it.

  return am;
}

void DataBase::OutputCatalog(const char* databasename)
{
  std::ofstream outfile(databasename);
    
    outfile << numberofrelations << std::endl;
    
    for(relations.GoHead(); relations.CurrPtr(); relations.GoNext()) 
    {
	Plan_Relation *r = relations.CurrPtr();
	
	outfile << r->Name() << " " <<  r->attributeList.Size()
	        << " " << r->Cardinality() << " " << r->TupleSize()
		    << " " << r->NumPages() << std::endl;
	
	Attribute *a;
	
	for (r->attributeList.GoHead();
	     (a = r->attributeList.CurrPtr());
	     r->attributeList.GoNext())
	{
	    outfile << "    " << a->Name() << " " << a->AttrID()
		<< " ";
	    outfile << a->Type();
	    outfile << a->minVal << " " << a->maxVal << " " << a->Size()
		<< " " << a->Offset();

	    int accessMethods = 0;
	    Plan_AccessMethod* am;
	    
	    // how many access methods for this attr?
	    for(r->accessMethodList.GoHead();
		(am = r->accessMethodList.CurrPtr());
		r->accessMethodList.GoNext())
	    {
		if ( am->AttributePtr() == a)
		    accessMethods++;
	    }
	    
	    outfile << " " <<  accessMethods << std::endl;
	    
	     for(r->accessMethodList.GoHead();
		(am = r->accessMethodList.CurrPtr());
		r->accessMethodList.GoNext())
	    {
		if ( am->AttributePtr() == a)
		{
		    outfile << "        " << am->Name() << " "
			<< am->NumLeafPages();
		    outfile << am->Order();
		    if (am->IndexPtr())
		    {
			outfile << "1 ";
			if (am->IndexPtr()->IsClustered())
			    outfile << "1 ";
			else
			    outfile << "0 ";
			outfile << am->IndexPtr()->DistinctKeys() << " ";
			outfile << am->IndexPtr()->NumPages() << " ";
			switch ( am->IndexPtr()->GetSelectType())
			{
			  case selRange:
			    outfile << "0 ";
			    break;
			  case selExact:
			    outfile << "1 ";
			    break;
			  case selBoth:
			    outfile << "2 ";
			    break;
			  default:
			    outfile << "3 ";
			    break;
			}
			outfile << am->IndexPtr()->Filename() << std::endl;	       
		    }		    
		    else 
			outfile << " 0" << std::endl;		    
		}
	    }
	}
	
    }
}

DataBase::~DataBase()
{
    for (relations.GoHead(); relations.CurrPtr(); relations.GoNext())
    {
	relations.CurrPtr()->accessMethodList.Clean();
	relations.CurrPtr()->attributeList.Clean();
    }

    relations.Clean();
}











