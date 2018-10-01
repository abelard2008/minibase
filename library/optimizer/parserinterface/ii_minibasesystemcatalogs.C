//
// ii_minibasesystemcatalogs.C
//
// Minibase System Catalogs that use the real catalogs.
//
// Luke Blanshard, Spring 1996
//
#include "ii_systemcatalogs.h"
#include "ext_sys_defs.h"
#include "maincatalog.h"

#include <ostream>
#include <stdlib.h>
#include <string.h>
  

ii_MinibaseSystemCatalogs::~ii_MinibaseSystemCatalogs()
{}


void ii_MinibaseSystemCatalogs::OutputCatalog( const char* filename )
{
    MINIBASE_CATALOGPTR->dumpCatalog( filename );
}

Plan_Relation *ii_MinibaseSystemCatalogs::GetPlan_Relation(const std::string& relName )
{
    RelDesc relDesc;
    AttrDesc* attrDescs = 0;
    IndexDesc* indexDescs = 0;
    if ( MINIBASE_RELCAT->getInfo( relName, relDesc ) != OK
         || MINIBASE_ATTRCAT->getRelInfo( relName, relDesc.attrCnt, attrDescs ) != OK
         || MINIBASE_INDCAT->getRelInfo( relName, relDesc.indexCnt, indexDescs ) != OK )
      {
        delete[] attrDescs;
        return 0;
      }

    unsigned tupleSize = 0;
    for ( int i=0; i < relDesc.attrCnt; ++i )
        tupleSize += attrDescs[i].attrLen;

    Plan_Relation* answer = new Plan_Relation( relName, 0, relDesc.numTuples,
                                               tupleSize, relDesc.numPages );

      // First add all the attributes.
    for ( int i=0; i < relDesc.attrCnt; ++i )
      {
        AttrDesc& ad = attrDescs[i];
        Attribute* attr = new Attribute( ad.attrName, answer, ad.attrType,
                                         ad.attrLen, ad.attrOffset, ad.attrPos );
        if ( ad.attrType == attrReal )
          {
            attr->minVal = ad.minVal.floatVal;
            attr->maxVal = ad.maxVal.floatVal;
          }
        else if ( ad.attrType == attrInteger )
          {
            attr->minVal = ad.minVal.intVal;
            attr->maxVal = ad.maxVal.intVal;
          }
        else
          {
            attr->minVal = ad.minVal.strVal;
            attr->maxVal = ad.maxVal.strVal;
          }
        answer->attributeList.InsertTail(attr);
      }

      // Then add all the indexes.
    for ( int i=0; i < relDesc.indexCnt; ++i )
      {
        IndexDesc& id = indexDescs[i];

        for ( answer->attributeList.GoHead();
              answer->attributeList.CurrPtr()
                  && answer->attributeList.CurrPtr()->Name() != id.attrName;
              answer->attributeList.GoNext() )
          {} // empty loop body

        Attribute* a = answer->attributeList.CurrPtr();
        if ( !a )
            continue;

        const char* amName = "???";
        SelectType st = selUndefined;
        int cost = 0;

        if ( id.accessType == B_Index )
          {
            amName = "B_Index";
            st = selBoth;
            cost = (sizeof(RID) + a->Size()) * id.distinctKeys / MINIBASE_PAGESIZE;
          }
        else if ( id.accessType == Hash )
          {
            amName = "Hash";
            st = selExact;
            cost = 1;   // Lifted these costs from `dump.C' in catalog directory.
          }

        char* name;
        MINIBASE_INDCAT->buildIndexName( id.relName, id.attrName, id.accessType, name );

        AttributeList al;
        al.InsertTail( *a );

        Index* index = new Index( id.clustered, id.distinctKeys, id.indexPages, st, al, name );
        Plan_AccessMethod* am = new Plan_AccessMethod( amName, cost, id.order, a, index );
        answer->accessMethodList.InsertTail(am);

        delete index;   // Apparently copied by Plan_AccessMethod constructor.
        delete name;
      }

      // Finally, add a special access method for scanning the whole file.
    answer->attributeList.GoHead();
    Plan_AccessMethod* am = new Plan_AccessMethod( "FileScan", relDesc.numPages, Random,
                                                   answer->attributeList.CurrPtr(), 0 );
    answer->accessMethodList.InsertTail(am);

    delete[] attrDescs;
    delete[] indexDescs;
    return answer;
}

bool ii_MinibaseSystemCatalogs::RelationExists( const std::string& relName )
{
    RelDesc relDesc;
    return MINIBASE_RELCAT->getInfo( relName, relDesc ) == OK;
}

Attribute* ii_MinibaseSystemCatalogs::GetAttribute( const std::string& relName, const std::string& attrName )
{
    AttrDesc ad;
    if ( MINIBASE_ATTRCAT->getInfo(relName,attrName,ad) != OK )
        return 0;
    return new Attribute( ad.attrName, 0, ad.attrType,
                          ad.attrLen, ad.attrOffset, ad.attrPos );
}

void ii_MinibaseSystemCatalogs::PrintRelations()
{
    Status status;
    RelDesc rel;
    RID rid;
    int recSize;
    Scan* scan = MINIBASE_RELCAT->openScan( status );

    while ( scan && scan->getNext(rid, (char*)MINIBASE_RELCAT->tuple, recSize) == OK )
      {
        MINIBASE_RELCAT->read_tuple( MINIBASE_RELCAT->tuple, rel );
        std::cout << rel.relName << ".:\n";
      }

    std::cout << "end - relations.:\n";
    delete scan;
}

void ii_MinibaseSystemCatalogs::PrintFilename()
{
    std::cout << "database\n" << MINIBASE_DBNAME << ".:\n";
}
