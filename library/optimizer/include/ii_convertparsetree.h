//
// ii_convertparsetree.h
//
// C++ Minirel Parser Interface
//
// Adapted by Stephen Harris from ii_convertparsetree.h written 
// for SHORE Persistent Object System Software.
//
/* $Id: ii_convertparsetree.h,v 1.2 1996/07/07 20:26:40 ramamurt Exp $
 * $Log: ii_convertparsetree.h,v $
 * Revision 1.2  1996/07/07 20:26:40  ramamurt
 * Version with ext_sys_defs fixed, heapfiles and log bug fixed, new interface for startup - and with recovery partially working in a shared-mem environment. Xaction abort works. Also modified concept of transaction. One process can start and commit many transactions but interleaved xactions are still not allowed
 *
 * Revision 1.1  1996/02/11 04:25:52  luke
 * Initial check-in of minibase
 *
 * Revision 1.1.1.1  1996/02/05 19:47:26  luke
 * Initial import of Minibase library into CVS.
 *
 * Revision 1.8  1995/09/01  21:13:15  michaell
 * pre revising front end check in
 *
 * Revision 1.7  1995/08/04  21:33:54  michaell
 * added having clause
 *
 * Revision 1.7  1995/08/04  21:33:54  michaell
 * added having clause
 *
 * Revision 1.6  1995/06/19  19:54:23  michaell
 * added some support for order by
 *
 * Revision 1.6  1995/06/19  19:54:23  michaell
 * added some support for order by
 *
 * Revision 1.5  1995/04/25  18:49:27  michaell
 * selects now properly choose to use an index appropriately.
 *
 * Revision 1.5  1995/04/25  18:49:27  michaell
 * selects now properly choose to use an index appropriately.
 *
 * Revision 1.4  1995/04/22  21:32:39  michaell
 * *** empty log message ***
 *
 * Revision 1.4  1995/04/22  21:32:39  michaell
 * *** empty log message ***
 *
 * Revision 1.3  1995/04/20  18:55:37  michaell
 * added routines to convert into CNF
 *
 * Revision 1.3  1995/04/20  18:55:37  michaell
 * added routines to convert into CNF
 *
 * Revision 1.2  1995/04/17  20:06:55  michaell
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

#ifndef _II_CONVERTPARSETREE_H
#define _II_CONVERTPARSETREE_H

#include "parser.h"
#include "query.h"

class ii_ConvertFromParseTree
{
private:
  State *state;

  // reading parse tree for a query.

  void DoQuery (pp_NODPTR); // a sql string we get can either be a query
  void DoUtility (pp_NODPTR); // or a utility (like create relation)

  void DoFrom (pp_NODPTR); // from clause
  void DoWhere (pp_NODPTR); // where clause
  void DoGroup (pp_NODPTR); // group by clause
  void DoHaving (pp_NODPTR); // do having by clause  
  void DoOrder (pp_NODPTR); // order by clause (not handled currently)
  int HasAggregateExpression (pp_NODPTR); // aggregate (not completely handled)

  void DoTargetList (pp_NODPTR); // the target list
  void DoTargetListItem (pp_NODPTR); // either a relationname or an aggregate
  void DoOrderList (pp_NODPTR); // the order list
  
  
  void DoPredicateTree (pp_NODPTR, SelectList &pl);
  AttrOperator NODE_TYPE_To_AttrOperator(pp_NODE_TYPE nt);
  SelectExpr *DoPredicate (pp_NODPTR node); // while others are selects
  Plan_Relation *GetFromRelation(pp_NODPTR relname);
  SelectOperand *GetOperandFromNode(pp_NODPTR node);

  Attribute *DoPredicateItem(pp_NODPTR node);
  Attribute *GetAttributeFromNode(pp_NODPTR node);
  Attribute *GetOrderAttribute(pp_NODPTR node);
  std::string GetIDName(pp_NODPTR node);
  
  AttrValue GetLiteralFromNode(pp_NODPTR node);
  void NotConversion(pp_NODPTR node, SelectList &pl);
  void OrConversion(pp_NODPTR node, SelectList &pl);
  
  // utilities.

  void DoCreateDatabase (pp_NODPTR);
  void DoOpenDatabase (pp_NODPTR);
  void DoCloseDatabase (pp_NODPTR);
  void DoCreateRelation (pp_NODPTR);
  void DoLoadRelation (pp_NODPTR);
  void DoCreateIndex (pp_NODPTR);

 
  // Transaction utilities

  void DoBeginTransaction (pp_NODPTR);
  void DoCommitTransaction (pp_NODPTR);
  void DoAbortTransaction (pp_NODPTR);
  void DoPrint(pp_NODPTR); //Dummy for testing purposes
public:
  ii_ConvertFromParseTree (pp_NODPTR parsetree);
  State *GetState() { return state; };
};

#endif



