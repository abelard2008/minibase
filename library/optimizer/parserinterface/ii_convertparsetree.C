//
// ii_convertparsetree.c
//
// C++ Minirel Parser Interface
//
// Adapted by Stephen Harris from ii_convertparsetree.c written 
// for SHORE Persistent Object System Software.
//
/* $Id: ii_convertparsetree.C,v 1.4 1996/07/07 20:26:53 ramamurt Exp $
 * $Log: ii_convertparsetree.C,v $
 * Revision 1.4  1996/07/07 20:26:53  ramamurt
 * Version with ext_sys_defs fixed, heapfiles and log bug fixed, new interface for startup - and with recovery partially working in a shared-mem environment. Xaction abort works. Also modified concept of transaction. One process can start and commit many transactions but interleaved xactions are still not allowed
 *
 * Revision 1.3  1996/05/23 03:51:59  luke
 * New heapfiles; rational approach to catalog use by optimizer
 *
 * Revision 1.2  1996/04/21 15:19:47  luke
 * Revamped space management; moved to gcc v2.7.1
 *
 * Revision 1.1  1996/02/11 04:27:06  luke
 * Initial check-in of minibase
 *
 * Revision 1.1.1.1  1996/02/05 19:47:40  luke
 * Initial import of Minibase library into CVS.
 *
 * Revision 1.8  1995/09/01  21:20:48  michaell
 * checkin before front end change.  group by & aggregates now work.
 *
 * Revision 1.6  1995/04/25  21:37:39  michaell
 * *** empty log message ***
 *
 * Revision 1.6  1995/04/25  21:37:39  michaell
 * *** empty log message ***
 *
 * Revision 1.5  1995/04/24  18:54:46  michaell
 * added support for the new range selectElem term, which modified how
 * new terms are added to the post-parsing tree
 *
 * Revision 1.5  1995/04/24  18:54:46  michaell
 * added support for the new range selectElem term, which modified how
 * new terms are added to the post-parsing tree
 *
 * Revision 1.4  1995/04/21  22:47:07  michaell
 * removed some debugging stuff
 *
 * Revision 1.4  1995/04/21  22:47:07  michaell
 * removed some debugging stuff
 *
 * Revision 1.3  1995/04/18  16:32:40  michaell
 * added support to convert trees into conjunctive normal form
 *
 * Revision 1.2  1995/04/17  20:07:40  michaell
 * added support for not
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
#include <ostream>
#include <stdlib.h>
#include <string.h>
#include <opt_globals.h>
#include <mr_syserr.h>

#include "ii_convertparsetree.h"
#include "ii_systemcatalogs.h"

extern SysErr ParserErr;

// functions imported from manuvir's typechk.c
// Attribute *find_attr_id(pp_NODPTR);  
// Attribute *find_attr_dot(pp_NODPTR); 

inline char *GetIdName (pp_NODPTR ident) {
  return ident->nodeval.id_val->id_name;
}

inline int GetIntegerValue (pp_NODPTR integer) {
  return integer->nodeval.int_val;
}



ii_ConvertFromParseTree::ii_ConvertFromParseTree(pp_NODPTR parsetree)
{
  state = new State();

  // reset node ID assignments (ID's are assigned only for display purposes)
  ResetNodeIDs();

  if (!state) FatalError(MEM);
  switch (parsetree->type)
    {
    case nt_query:
      DoQuery (parsetree);
      break;
    case nt_utility:
      DoUtility (parsetree);
      break;
    default:
//      std::cerr << "ii_ConvertFromParseTree: neither query nor utility" << endl;
//      exit (1);
      ParserErr.SetState(SysErr::PNotQueryNotUtility);
      break;
    }
}

void
ii_ConvertFromParseTree::DoQuery (pp_NODPTR q)
{
  q = q->nodeleftchild;
  if (q->type != nt_query_stmt)
    {
//      std::cerr << "ii_ConvertFromParseTree: cannot handle set operations" << endl;
//      exit (1);
       ParserErr.SetState(SysErr::PSetOp);     
    }

  DoFrom       (q = q->nodeleftchild);
  DoTargetList (q = q->nodenext);
  DoWhere      (q = q->nodenext);
  DoGroup      (q = q->nodenext);
  DoHaving     (q = q->nodenext);  
  DoOrder      (q = q->nodenext);

  return;
}


void ii_ConvertFromParseTree::DoFrom (pp_NODPTR f)
{
  if (f->type != nt_from)
    {
//      std::cerr << "DoFrom: not a from\n";
//      exit (1);
       ParserErr.SetState(SysErr::PNotFrom);     
    }
  

  pp_NODPTR relname;
  for (relname = f->nodeleftchild; relname; relname = relname->nodenext) {
    state->joins.InsertHead(GetFromRelation(relname));
  }
}


Plan_Relation *ii_ConvertFromParseTree::GetFromRelation(pp_NODPTR relname)
{
  char	  *from_rel;	/* relation in From clause */
  char    *rel_name;

  if(relname->type == nt_rel_name) {
    from_rel = relname->nodeleftchild->nodeval.id_val->id_name;
    rel_name = relname->noderightchild->nodeval.id_val->id_name;
  }
  else	{
    from_rel = relname->nodeval.id_val->id_name;
    rel_name = from_rel;
  }

  Plan_Relation* result = systemCatalogs->GetPlan_Relation(from_rel);
  result->SetName(rel_name);

  return result;
}

// that interests me in the select clause is whether or
// not there is any aggregate expression.
// Note that during optimization i am not interested in knowing 
// which aggregates are applied to which attributes.
// just the presence (absence) is enough for producing
// the optimal query plan.
// of course this information will be required later when
// converting the optimal plan (which has incomplete information;
// statements like "apply all applicable aggregates at this point")
// into gamma packets (which require complete information).
void ii_ConvertFromParseTree::DoTargetList(pp_NODPTR s)
{
  if (s->type != nt_select)
    {
//      std::cerr << "DoTargetList: not a select\n";
//      exit (1);
       ParserErr.SetState(SysErr::PNotSelect);     
    }

  if (s->nodeleftchild->type == nt_distinct)
      state->distinct = true;
  else
      state->distinct = false;
  
  switch (s->noderightchild->type)
    {
    case nt_star:
      // doesnt have any aggregates
      for (state->joins.GoHead(); state->joins.CurrPtr(); 
	   state->joins.GoNext()) 
      {	  
	  state->project + &state->joins.CurrPtr()->attributeList;
	  state->neededAttributes + &state->joins.CurrPtr()->attributeList;
      }
      
    break;
      
    default:
      // there is a select list
      // see if there are any aggregate expressions in the list.
      for (pp_NODPTR tmp = s->nodeleftchild->nodenext; tmp; 
	   tmp = tmp->nodenext)
	DoTargetListItem(tmp);
    }
  
  return;
}


Attribute::AggregateType ToAgg(pp_NODE_TYPE type)
{
    switch (type)
    {
      case nt_count:
	return Attribute::COUNT;
      case nt_avg:
	return Attribute::AVG;
      case nt_max_nod:
	return Attribute::MAX;
      case nt_min_nod:
	return Attribute::MIN;
      case nt_sum:
	return Attribute::SUM;
      default:
	return Attribute::NONE;	
    }
}


void ii_ConvertFromParseTree::DoTargetListItem(pp_NODPTR node)
{
  switch (node->type)
    {
    case nt_dot:
    case nt_id:
	state->neededAttributes.InsertHead(GetAttributeFromNode(node));	
	state->project.InsertHead(GetAttributeFromNode(node));
      break;

    case nt_count: 
      if (node->nodeleftchild && node->nodeleftchild->type == nt_star)
      {
	  state->joins.GoHead(); 
	  state->joins.CurrPtr()->attributeList.GoHead();
	  Attribute *att = new Attribute(*state->joins.CurrPtr()->attributeList.CurrPtr());
	  att->MakeAgg( ToAgg(node->type), false);
	  state->project.InsertHead(att);

	  /*
	  for (state->joins.GoHead(); state->joins.CurrPtr(); 
	       state->joins.GoNext())
	      state->project + &state->joins.CurrPtr()->attributeList;
	      */
	  state->containsAggregates = true;
      }
      else if (node->noderightchild )
      {
	  state->containsAggregates = true;
	// count all distinct column  
	  Attribute *att = new Attribute(*GetAttributeFromNode(node->noderightchild));
	  state->neededAttributes.InsertHead(*att);
	
	  if (!att) ParserErr.SetState(SysErr::PAggregate);
	  else 
	  {	         
	      att->MakeAgg( ToAgg(node->type), true);
	      state->project.InsertHead(att);
	  }	  
      }
      else
	  ParserErr.SetState(SysErr::PAggregate);     
      break;
      
    case nt_avg:
    case nt_max_nod:
    case nt_min_nod:
    case nt_sum:
	state->containsAggregates = true;
	if (node->nodeleftchild && node->noderightchild)
	{
	    Attribute *att = new Attribute (*GetAttributeFromNode(node->noderightchild));
	    if (att->Type() != attrInteger && att->Type() != attrReal)
	    {
		delete att;
		ParserErr.SetState(SysErr::PIntOrFloatForAgg);
	    }

	    state->neededAttributes.InsertHead(*att);
	    if (!att) 
		ParserErr.SetState(SysErr::PAggregate);
	    else 
	    {	
		att->MakeAgg( ToAgg(node->type), 
			     node->nodeleftchild->type == nt_distinct);
		if (!state->aggDistinct)
		{
		    state->aggDistinct
			= (node->nodeleftchild->type == nt_distinct);
		}
		state->project.InsertHead(att);
	    }	  
	}      
	else
	{	  
	    ParserErr.SetState(SysErr::PAggregate);
	}      
	break; 
	
      default:

//      DoTargetListItem(node->noderightchild); 
 //      break;
      
//      std::cerr << "HasAggregateExpression: error. this case not handled" << endl;
//      exit (1);
	ParserErr.SetState(SysErr::PAggregate);     
	break;
    }
}
     
void ii_ConvertFromParseTree::DoWhere (pp_NODPTR w)
{
  if (w->type == nt_null)
    {
      return;
    }

  if (w->type != nt_where)
    {
//      std::cerr << "DoWhere: not a where\n";
//      exit (1);
       ParserErr.SetState(SysErr::PNotWhere);     
    }

  DoPredicateTree(w->nodeleftchild, state->selects);

  return;
}


void 
ii_ConvertFromParseTree::DoPredicateTree (pp_NODPTR predicatetree, SelectList& predicateList)
{
  // Attribute *a1, *a2;
  Select *sel;
    
  switch (predicatetree->type)
    {
    case nt_and:
	DoPredicateTree (predicatetree->nodeleftchild, predicateList);	
	DoPredicateTree (predicatetree->noderightchild, predicateList);
      break;
     
    case nt_not:
      NotConversion( predicatetree, predicateList );
      break;
      
    case nt_or:
      OrConversion( predicatetree, predicateList );
      break;
      
    case nt_not_equal:
    case nt_equal:
	sel = new Select(DoPredicate(predicatetree));
	predicateList.InsertHead(sel);
	break;
	
    case nt_less:
    case nt_greater:
    case nt_less_eq:
    case nt_greater_eq: 
      {
	sel = new Select(DoPredicate(predicatetree));
	bool add = TRUE;

// the following code has been ifd out because the minirel layers don't
// support full ranges; just one ended ones.  
#if 0	
	if (sel && sel->GetType() == selRange)
	{
	    SelectElem *elem = sel->GetExpr()->GetElem();

	    if (   (elem->Arg1() && elem->Arg1()->IsLiteral())
		|| (elem->Arg3() && elem->Arg3()->IsLiteral()))
	    {
		for( predicateList.GoHead(); add && predicateList.CurrPtr();
		    predicateList.GoNext())
		{
		    SelectExpr* expr = predicateList.CurrPtr()->GetExpr();
		    
		    if ( expr && expr->IsElement()
			&& expr->GetElem()->TryToInclude(*elem) )
			add = FALSE;
		} // for
	    } // only "concrete" comparisons
	} // only for simple ranges
#endif	
	if (add)
	    predicateList.InsertHead(sel);
	break;
      }
    default:
//      std::cerr << "DoPredicateree: this case not handled\n";
//      exit (1);
      ParserErr.SetState(SysErr::PBadOperator);     
      break;
    }

  return;
}

void
ii_ConvertFromParseTree::NotConversion(pp_NODPTR predicatetree, SelectList& predicateList)
{
    pp_AST_NODE revisedcenter;
    pp_AST_NODE revisedleft;
    pp_AST_NODE revisedright;
    
    pp_NODPTR notTerm = predicatetree->nodeleftchild;
       
    if (predicatetree->type != nt_not || !notTerm )
	// it's wrong!
    {
	ParserErr.SetState(SysErr::PBadOperator);     
	return;
    }
    
    revisedcenter = *notTerm; // a surface (not deep) copy
    revisedcenter.nodeparent = predicatetree->nodeparent;
    // logically, this is where it belongs.  Don't think it's relevant
    
    switch( notTerm->type )
    {
      case nt_or: // not a or b <=> not a and not b
	revisedcenter.type = nt_and;
	revisedleft.type = nt_not;
	revisedright.type = nt_not;
	revisedright.nodeparent = revisedleft.nodeparent = &revisedcenter;
	revisedcenter.nodeleftchild = &revisedleft;
	revisedcenter.noderightchild = &revisedright;
	revisedleft.nodeleftchild = revisedleft.noderightchild = notTerm->nodeleftchild;
	revisedright.nodeleftchild = revisedright.noderightchild = notTerm->noderightchild;
	DoPredicateTree(&revisedcenter, predicateList);
	break;
      case nt_and: // not a and b <=> not a or not b
	revisedcenter.type = nt_or;
	revisedleft.type = nt_not;
	revisedright.type = nt_not;
	revisedright.nodeparent = revisedleft.nodeparent = &revisedcenter;
	revisedcenter.nodeleftchild = &revisedleft;
	revisedcenter.noderightchild = &revisedright;
	revisedleft.nodeleftchild = revisedleft.noderightchild = notTerm->nodeleftchild;
	revisedright.nodeleftchild = revisedright.noderightchild = notTerm->noderightchild;
	DoPredicateTree(&revisedcenter, predicateList);
	break;
      case nt_not: // not not a <=> a
	DoPredicateTree(notTerm->nodeleftchild, predicateList);
	break;
	
      case nt_not_equal: // not a !=b <=> a = b
	revisedcenter.type = nt_equal;
	DoPredicateTree(&revisedcenter, predicateList);	
	break;
      case nt_less: // not a < b <=> a >= b
	revisedcenter.type = nt_greater_eq;
        DoPredicateTree(&revisedcenter, predicateList);	
	break;
      case nt_greater: // not a > b <=> a <= b
	revisedcenter.type = nt_less_eq;
	DoPredicateTree(&revisedcenter, predicateList);	
      	break;
      case nt_less_eq: // not a <= b <=> a > b
	revisedcenter.type = nt_greater;
	DoPredicateTree(&revisedcenter, predicateList);	
      	break;
      case nt_greater_eq: // not a >=b <=> a < b
	revisedcenter.type = nt_less;
	DoPredicateTree(&revisedcenter, predicateList);	
      	break;
      case nt_equal: // not a == b <=> a != b
	revisedcenter.type = nt_not_equal;
	DoPredicateTree(&revisedcenter, predicateList);	
	break;
      default:
	ParserErr.SetState(SysErr::PBadOperator);     
	break;
    }
    
    return;
    
}

void
ii_ConvertFromParseTree::OrConversion(pp_NODPTR predicatetree, SelectList& predicateList)
{
    SelectList leftDisjunct;    
    SelectList rightDisjunct;

    Select *lcurrent; // left predicate list
    Select *rcurrent; // right predicate list

    Select *newSelect; // pointer to one to add

    DoPredicateTree(predicatetree->nodeleftchild, leftDisjunct);
    DoPredicateTree(predicatetree->noderightchild, rightDisjunct);
    
    // we now will add to the returning list the disjoins of all of the members of left with right 
    for (leftDisjunct.GoHead(); (lcurrent = leftDisjunct.CurrPtr()); leftDisjunct.GoNext())
	for ( rightDisjunct.GoHead(); (rcurrent = rightDisjunct.CurrPtr()); rightDisjunct.GoNext())
	{
	    // std::cout << "Left: ";
	    // lcurrent->Show();
	    // std::cout << endl;

	    // std::cout << "Right: ";
	    // rcurrent->Show();
	    // std::cout << endl;

	    SelectExpr *addme = new SelectExpr();
	    // std::cout << "Test or part: "; 
	    SelectExpr *left = new SelectExpr(*lcurrent->GetExpr());
	    SelectExpr *right = new SelectExpr(*rcurrent->GetExpr());
	    
            *addme = new SelectTerm (lopOR, left, right);
	    // addme->Show(); 
	    
	
	    newSelect = new Select(addme);
	    // newSelect->Show();
	    // std::cout << endl;
	    
	    predicateList.InsertHead(newSelect);	    
	}
}



AttrOperator ii_ConvertFromParseTree::NODE_TYPE_To_AttrOperator(pp_NODE_TYPE nt)
{
  switch (nt)
    {
    case nt_equal:       return(aopEQ); break;
    case nt_not_equal:   return(aopNE); break;
    case nt_less:        return(aopLT); break;
    case nt_greater:     return(aopGT); break;
    case nt_less_eq:     return(aopLE); break;
    case nt_greater_eq:  return(aopGE); break;
    default:             return(aopNOP); break;
    }
}



SelectExpr *ii_ConvertFromParseTree::DoPredicate (pp_NODPTR predicate)
{
  SelectExpr *result = new SelectExpr();

  switch (predicate->type)
    {
    case nt_or: 
      *result = new SelectTerm(lopOR, DoPredicate(predicate->nodeleftchild),
			       DoPredicate(predicate->noderightchild));
      break;

    case nt_equal: case nt_not_equal: case nt_less:
    case nt_greater: case nt_less_eq: case nt_greater_eq:
    case nt_dot: case nt_id:
      {
        AttrOperator op = NODE_TYPE_To_AttrOperator(predicate->type);
        if (op == aopNOP)
            *result = new SelectElem(op, GetOperandFromNode(predicate), NULL);
        else 
            *result = new SelectElem(op, GetOperandFromNode(predicate->nodeleftchild), 
                                     GetOperandFromNode(predicate->noderightchild));
/*      result->IsElement() = TRUE;
        result->GetElem()->GetOperator() = NODE_TYPE_To_AttrOperator(predicate->type);
        if (result->GetElem()->GetOperator() == aopNOP)
        result->GetElem()->Arg1() = GetOperandFromNode(predicate);
        else {
        result->GetElem()->Arg1() = GetOperandFromNode(predicate->nodeleftchild);
        result->GetElem()->Arg2() = GetOperandFromNode(predicate->noderightchild);
        } */
        break;
      }
    default:
      break;
    }  

  return result;
}



void 
ii_ConvertFromParseTree::DoGroup (pp_NODPTR g)
{
   if (g->type == nt_null)
   {
       return; 
   }
   
  if (g->type != nt_group)
  {
      ParserErr.SetState(SysErr::PNotGroup);     
  }
  else 
  {
      pp_NODPTR gb = g->nodeleftchild;
      Attribute *a;
      
      while(gb)
      {
	  a = GetAttributeFromNode(gb);
	  gb = gb->nodenext;
	  state->groupby.InsertTail(a);
      }
  }
   
  return;
}

void 
ii_ConvertFromParseTree::DoHaving (pp_NODPTR g)
{
    if (g->type == nt_null)
	return; 

    if (g->type != nt_having)
    {
	//      std::cerr << "DoWhere: not a where\n";
	//      exit (1);
	ParserErr.SetState(SysErr::PNotHaving);  
	return;
    }
    
    if (state->groupby.IsEmpty())
    {	
	ParserErr.SetState(SysErr::PNeedGroupBy);
	return;
    }
    
    DoPredicateTree(g->nodeleftchild, state->having);
    
//    ParserErr.SetState(SysErr::PHaving);     
    
    return;
}

void 
ii_ConvertFromParseTree::DoOrder (pp_NODPTR o)
{
  if (o->type == nt_null)
    return;

  if (o->type != nt_order)
  {
//      std::cerr << "DoOrder: not a order\n";
//      exit (1);
      ParserErr.SetState(SysErr::PNotOrder);     
  }

  DoOrderList ( o->nodeleftchild );
   
//  std::cerr << "error: order by clause not yet handled" << endl;
//  exit (1);
//  ParserErr.SetState(SysErr::POrder);     

  return;
}

void
ii_ConvertFromParseTree::DoOrderList (pp_NODPTR ol)
{
    OrderAttr* oa;

    while (ol)
    {
	oa = new OrderAttr;
	oa->attr = GetOrderAttribute(ol);
        for (state->project.GoHead();
	     state->project.CurrPtr() 
	     && *(oa->attr) != *(state->project.CurrPtr());
	     state->project.GoNext())  /* do nothing */ ;
	
	if ( !state->project.CurrPtr())
	{
	    ParserErr.SetState(SysErr::PNotSelected);	    
	}
	
	ol = ol->nodenext;
	if ( ol && ol->type == nt_dsc )
	    oa->tupleOrder = Descending;
	else
	    oa->tupleOrder = Ascending;
	state->order.InsertTail(oa);
	if (ol) ol = ol->nodenext;	
    }
    return;
}

Attribute *
ii_ConvertFromParseTree::GetOrderAttribute(pp_NODPTR node)
{
  std::string rel_name;	/* relation or var name */
  std::string attr_name;	/* attrib name*/ 
  Plan_Relation *prRel; /* relation in From clause */

  switch (node->type)
  {
    case nt_dot:
      rel_name = node->nodeleftchild->nodeval.id_val->id_name;
      attr_name = GetIDName(node->noderightchild);
      
      // attr_name = node->noderightchild->nodeval.id_val->id_name;
      for (state->joins.GoHead(); 
	   state->joins.CurrPtr() && rel_name != state->joins.CurrPtr()->Name();
	   state->joins.GoNext()) {}

      if ( (prRel = state->joins.CurrPtr()) ) {
	for (prRel->attributeList.GoHead();
	     prRel->attributeList.CurrPtr() && attr_name != prRel->attributeList.CurrPtr()->Name();
	     prRel->attributeList.GoNext()) {}
	return prRel->attributeList.CurrPtr();
      }

//      return find_attr_dot(node);
      break;
      
    case nt_id:
      attr_name = GetIDName(node);
      //      attr_name = node->nodeval.id_val->id_name;
      for (state->joins.GoHead(); state->joins.CurrPtr(); state->joins.GoNext()) {
	prRel = state->joins.CurrPtr();
	for (prRel->attributeList.GoHead();
	     prRel->attributeList.CurrPtr() && attr_name != prRel->attributeList.CurrPtr()->Name();
	     prRel->attributeList.GoNext()) {}
	if (prRel->attributeList.CurrPtr())
	  return prRel->attributeList.CurrPtr();
      }


//      return find_attr_id (node);
      break;

    default:
      break;
    }
  return NULL; // it is not an attribute.
}

std::string
ii_ConvertFromParseTree::GetIDName( pp_NODPTR node)
{
  std::string rv;
    int id;
    AttributeList& proj = state->project;
    
    switch ( node->type )
    {
      case nt_intlit:
	id = node->nodeval.int_val;
	for( proj.GoHead(); --id && proj.CurrPtr(); proj.GoNext())
	    /* do nothing */ ;
	
	rv = proj.CurrPtr()->Name();
	
	break;
     
      case nt_id:
	rv = node->nodeval.id_val->id_name;	
	break;
       
      default:
	rv = "";	
	break;	
    }
    
    return rv;
    
}

void 
ii_ConvertFromParseTree::DoUtility (pp_NODPTR u)
{
  u = u->nodeleftchild;

  switch (u->type)
    {
    case nt_create_db:
      DoCreateDatabase (u);
      break;
    case nt_open_db:
      DoOpenDatabase (u);
      break;
    case nt_close_db:
      DoCloseDatabase (u);
      break;

    case nt_create_table:
      DoCreateRelation (u);
      break;
    case nt_load_table:
      DoLoadRelation (u);
      break;
    case nt_create_index:
      DoCreateIndex (u);
      break;
    // Added for transaction capability RR, June 19 1996
    case nt_xbegin:
      DoBeginTransaction(u); //check for validity of transaction
      break;
    case nt_xcommit:
      DoCommitTransaction(u); //check for validity of transaction
      break;
    case nt_xabort:
      DoAbortTransaction(u); //abort the given transaction
      break;
    // End of additions.. RR
    case nt_drop_db:
    case nt_drop_table:
    case nt_drop_index:
    default:
//      std::cerr << "ii_ConvertFromParseTree: this utility not handled yet" << endl;
//      exit (1);
      ParserErr.SetState(SysErr::PBadUtility);     
     break;
    }
}

void
ii_ConvertFromParseTree::DoCreateDatabase (pp_NODPTR /*cdb*/)
{
// std::cerr << "ii_ConvertFromParseTree::DoCreateDatabase not handled yet" << endl;
// exit (1);
   ParserErr.SetState(SysErr::PBadUtility);     
}


void
ii_ConvertFromParseTree::DoOpenDatabase (pp_NODPTR /*odb*/)
{
// std::cerr << "ii_ConvertFromParseTree::DoOpenDatabase not handled yet" << endl;
// exit (1);
  ParserErr.SetState(SysErr::PBadUtility);     
}


void
ii_ConvertFromParseTree::DoCloseDatabase (pp_NODPTR /*cdb*/)
{
// std::cerr << "ii_ConvertFromParseTree::DoCloseDatabase not handled yet" << endl;
// exit (1);
  ParserErr.SetState(SysErr::PBadUtility);     
}


void 
ii_ConvertFromParseTree::DoCreateRelation (pp_NODPTR /*cr*/)
{
/*  ii_query->utilityoperator->utilitytype = ii_UtilityOperator::CREATE_RELATION;
  
  char *relationname = GetIdName (cr->nodeleftchild);

  // now the attributes
  pp_NODPTR col_data_type_list = cr->nodeleftchild->nodenext;

  // calculate the number of attribute
  pp_NODPTR n=col_data_type_list;
  
  for (int numberofattributes = 0; n->type != nt_key_nod; numberofattributes++)
    {
      n = n->nodenext; // skip over the attributename
      n = n->nodenext; // skip over the data type
      if (n->type == nt_null) 
	n = n->nodenext; // skip over not_null part if any
    }

  ii_query->utilityoperator->relationdescriptor = 
    new ii_UtilityOperator::RelationDescriptor (relationname, numberofattributes);

  // now another pass over the attributes to make the attribute array
  n = col_data_type_list;
  for (int attnum = 0; n->type != nt_key_nod; attnum++)
    {
      char *attname = GetIdName (n);
      n = n->nodenext;
      switch (n->type)
	{
	case nt_character_typ:
	  ii_query->utilityoperator->relationdescriptor->
	    SetNthAttribute (attnum, attname, 
			     GetIntegerValue (n->nodeleftchild), _StringAdt);
	  break;
	case nt_integer_typ:
	  ii_query->utilityoperator->relationdescriptor->
	    SetNthAttribute (attnum, attname, 
			     2, _Int4Adt);
	  break;
	case nt_float_typ:
	  ii_query->utilityoperator->relationdescriptor->
	    SetNthAttribute (attnum, attname, 
			     4, _RealAdt);
	  break;

	default:
//	  std::cerr << "ii_ConvertFromParseTree: unknown attribute type" << endl;
//	  exit (1);
          ParserErr.SetState(SysErr::PBadBadAttrType);     
	  break;
	}
      n = n->nodenext;
      if (n->type == nt_null)
	n = n->nodenext; 
    }
*/
// std::cerr << "ii_ConvertFromParseTree::DoCreateRelation not handled yet" << endl;
// exit (1);
  ParserErr.SetState(SysErr::PBadUtility);     
}

void
ii_ConvertFromParseTree::DoLoadRelation (pp_NODPTR /*lr*/)
{
// std::cerr << "ii_ConvertFromParseTree::DoLoadRelation not handled yet" << endl;
// exit (1);
  ParserErr.SetState(SysErr::PBadUtility);     
}

// Starting Transaction Additions.. RR
void
ii_ConvertFromParseTree::DoBeginTransaction (pp_NODPTR )
{
  ParserErr.SetState(SysErr::PBadUtility);     
}

void
ii_ConvertFromParseTree::DoCommitTransaction (pp_NODPTR )
{
  ParserErr.SetState(SysErr::PBadUtility);     
}

void
ii_ConvertFromParseTree::DoAbortTransaction (pp_NODPTR )
{
  ParserErr.SetState(SysErr::PBadUtility);     
}

void
ii_ConvertFromParseTree::DoPrint (pp_NODPTR )
{
  ParserErr.SetState(SysErr::PBadUtility);     
}

// Ending Transaction additions - Ranjani

void
ii_ConvertFromParseTree::DoCreateIndex (pp_NODPTR /*ci*/)
{
  // assume that only one attribute has been given
/*  pp_NODPTR attribute = ci->noderightchild->nodeprev;

  if (attribute->type != nt_id)
    {
//      std::cerr << "ii_ConvertFromParseTree: integer for attribute not handled" 
//	<< endl;
//      exit (1);
        ParserErr.SetState(SysErr::PIntForAttr);     
    }

  ii_query->utilityoperator->attributename = GetIdName (attribute);

  if (ci->nodeleftchild->type == nt_uniq)
    {
      ii_query->utilityoperator->utilitytype = 
	ii_UtilityOperator::CREATE_UNIQUE_INDEX;
      ii_query->utilityoperator->relationname = 
	GetIdName (ci->nodeleftchild->nodenext->nodenext);
    }
  else
    {
      ii_query->utilityoperator->utilitytype = ii_UtilityOperator::CREATE_INDEX;
      ii_query->utilityoperator->relationname = 
	GetIdName (ci->nodeleftchild->nodenext);
    }
*/
// std::cerr << "ii_ConvertFromParseTree::DoCreateIndex not handled yet" << endl;
// exit (1);
  ParserErr.SetState(SysErr::PBadUtility);     
}
    

SelectOperand *ii_ConvertFromParseTree::GetOperandFromNode(pp_NODPTR node)
{
    SelectOperand *result = new SelectOperand();
    
    if (!result) FatalError(MEM);
    switch (node->type)
    {
      case nt_dot:
      case nt_id:
      case nt_count:
      case nt_avg:
      case nt_max_nod:
      case nt_min_nod:
      case nt_sum:
	*result = DoPredicateItem(node);
        break;
      default:
	*result = GetLiteralFromNode(node);
	break;
    }
    
    return result;
}

Attribute*
ii_ConvertFromParseTree::DoPredicateItem(pp_NODPTR node)
{
    switch (node->type)
    {
      case nt_dot:
      case nt_id:
	return GetAttributeFromNode(node);
      break;

    case nt_count: 
      if (node->nodeleftchild && node->nodeleftchild->type == nt_star)
      {
	  state->joins.GoHead(); 
	  state->joins.CurrPtr()->attributeList.GoHead();
	  Attribute *att = new Attribute(*state->joins.CurrPtr()->attributeList.CurrPtr());
	  att->MakeAgg( ToAgg(node->type), false);
	  return att;
	  
	  /*
	  for (state->joins.GoHead(); state->joins.CurrPtr(); 
	       state->joins.GoNext())
	      state->project + &state->joins.CurrPtr()->attributeList;
	      */
	  state->containsAggregates = true;
      }
      else if (node->noderightchild )
      {
	  state->containsAggregates = true;
	// count all distinct column  
	  Attribute *att = GetAttributeFromNode(node->noderightchild);	
	  if (!att) ParserErr.SetState(SysErr::PAggregate);
	  else 
            {	         
              att = new Attribute(*att);
	      att->MakeAgg( ToAgg(node->type), true);
	      return att;	      
            }	  
      }
      else
	  ParserErr.SetState(SysErr::PAggregate);     
      break;
      
    case nt_avg:
    case nt_max_nod:
    case nt_min_nod:
    case nt_sum:
	state->containsAggregates = true;
	if (node->nodeleftchild && node->noderightchild)
	{
	    Attribute *att = new Attribute (*GetAttributeFromNode(node->noderightchild));
	    if (!att) 
		ParserErr.SetState(SysErr::PAggregate);
	    else 
	    {	
		att->MakeAgg( ToAgg(node->type), 
			     node->nodeleftchild->type == nt_distinct);
		if (!state->aggDistinct)
		{
		    state->aggDistinct
			= (node->nodeleftchild->type == nt_distinct);
		}
		return att;
	    }	  
	}      
	else
	{	  
	    ParserErr.SetState(SysErr::PAggregate);
	}      
	break; 
	
      default:

//      DoTargetListItem(node->noderightchild); 
 //      break;
      
//      std::cerr << "HasAggregateExpression: error. this case not handled" << endl;
//      exit (1);
	ParserErr.SetState(SysErr::PAggregate);     
	break;
    }
    return NULL;
}



Attribute *
ii_ConvertFromParseTree::GetAttributeFromNode(pp_NODPTR node)
{
  std::string rel_name;	/* relation or var name */
  std::string attr_name;	/* attrib name*/ 
  Plan_Relation *prRel; /* relation in From clause */

  switch (node->type)
    {
    case nt_dot:
      rel_name = node->nodeleftchild->nodeval.id_val->id_name;
      attr_name = node->noderightchild->nodeval.id_val->id_name;
      for (state->joins.GoHead(); 
	   state->joins.CurrPtr() && rel_name != state->joins.CurrPtr()->Name() ;
	   state->joins.GoNext()) {}

      if ( (prRel = state->joins.CurrPtr()) ) {
	for (prRel->attributeList.GoHead();
	     prRel->attributeList.CurrPtr() && attr_name != prRel->attributeList.CurrPtr()->Name();
	     prRel->attributeList.GoNext()) {}
	return prRel->attributeList.CurrPtr();
      }

//      return find_attr_dot(node);
      break;
      
    case nt_id:
      attr_name = node->nodeval.id_val->id_name;
      for (state->joins.GoHead(); state->joins.CurrPtr(); state->joins.GoNext()) {
	prRel = state->joins.CurrPtr();
	for (prRel->attributeList.GoHead();
	     prRel->attributeList.CurrPtr() && attr_name != prRel->attributeList.CurrPtr()->Name();
	     prRel->attributeList.GoNext()) {}
	if (prRel->attributeList.CurrPtr())
	  return prRel->attributeList.CurrPtr();
      }


//      return find_attr_id (node);
      break;

    default:
      break;
    }
  return NULL; // it is not an attribute.

}


AttrValue ii_ConvertFromParseTree::GetLiteralFromNode(pp_NODPTR node)
{
  AttrValue result;

  switch (node->type) {
    case nt_intlit:   result = node->nodeval.int_val; break;
    case nt_floatlit: result = *node->nodeval.float_val; break;
  case nt_strlit:   result = std::string(node->nodeval.str_val); break;
  }
  return result;
}





