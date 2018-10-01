//
// ii_systemcatalogs.c
//
// C++ Minirel System Catalogs
//
// Adapted by Stephen Harris from ii_systemcatalogs.c written 
// for SHORE Persistent Object System Software.
//


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
#include "ii_systemcatalogs.h"

#include <ostream>
#include <stdlib.h>
#include <string.h>
  
ii_SystemCatalogs::~ii_SystemCatalogs()
{}


Plan_Relation *ii_BogusSystemCatalogs::GetPlan_Relation( const std::string& relName )
{
    for ( database.relations.GoHead();
          database.relations.CurrPtr() && database.relations.CurrPtr()->Name() != relName; 
          database.relations.GoNext() )
      {} // empty loop body

    if ( database.relations.CurrPtr() )
        return new Plan_Relation( *database.relations.CurrPtr() );
    return 0;
}

bool ii_BogusSystemCatalogs::RelationExists( const std::string& relName )
{
    for ( database.relations.GoHead();
          database.relations.CurrPtr() && database.relations.CurrPtr()->Name() != relName; 
          database.relations.GoNext() )
      {} // empty loop body

    return database.relations.CurrPtr() != 0;
}

Attribute* ii_BogusSystemCatalogs::GetAttribute( const std::string& relName, const std::string& attrName )
{
    for ( database.relations.GoHead();
          database.relations.CurrPtr() && database.relations.CurrPtr()->Name() != relName; 
          database.relations.GoNext() )
      {} // empty loop body

    if ( !database.relations.CurrPtr() )
        return 0;

    Plan_Relation* r = database.relations.CurrPtr();
    for ( r->attributeList.GoHead();
          r->attributeList.CurrPtr() && r->attributeList.CurrPtr()->Name() != attrName;
          r->attributeList.GoNext() )
      {} // empty loop body

    if ( r->attributeList.CurrPtr() )
        return new Attribute( *r->attributeList.CurrPtr() );
    return 0;
}

void ii_BogusSystemCatalogs::PrintRelations()
{
    for ( database.relations.GoHead(); database.relations.CurrPtr(); database.relations.GoNext() )
	std::cout << database.relations.CurrPtr()->Name() << ".:" << std::endl;

    std::cout << "end - relations.:" << std::endl;
}

void ii_BogusSystemCatalogs::PrintFilename()
{
    std::cout << "catalog\n" << database.filename << ".:\n";
}


ii_SystemCatalogs* systemCatalogs = 0;


