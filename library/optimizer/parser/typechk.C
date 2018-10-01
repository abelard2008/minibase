/*******************************************************************************
 *                                                                              *
 *      SQL Front-End Interactive System for Gamma & Exodus                     *
 *                                                                              *
 *                  (c) Dec. 1992 - Edward Hill, University of Wisconsin        *
 *                                                                              *
 *      Permission is granted to copy all or part of this code, provided this   *
 *      header is present in its entirety in the copied version of the code.    *
 *                                                                              *
 *      Credits : Manuvir Das                                                   *
 *                                                                              *
 *******************************************************************************/


/*****************************************************************************
 *
 * Modified by Stephen Harris 5/94
 *
 * Summary of changes:
 *
 *      1) Changed all routines to use new ii_systemcatalogs.  These
 *         catalogs return pointers to instances of the Plan_Relation and
 *         Attribute classes declared in query.h.
 *
 *      2) Removed all uses of pp_SYMBOL_ENTRY because it is no longer
 *         needed due to the above modification to the system catalogs.
 *    
 *      3) Updated existing documentation.
 *
 *****************************************************************************/


/*******************************************************************************
 *                                                                              *
 *      typechk.c : the SQL type checking routines                              *
 *                                                                              *
 *******************************************************************************/

/*********************** typechk.c *****************************

  After yacc builds the parse tree, there is a pass made
  over it which removes all NOT nodes. After this another pass is
  made over the tree again to collapse the AND nodes. At this point
  pp_typechk() is called.

  The purpose of the typechecker is twofold: 
  1. To make sure that the current state of the parse tree is
  semantically well-formed.
  2. To decorate references in the tree to attributes with information
  from the catalogs to be use later in the operator tree.  All 
  attributes are decorated with a pointer to the attributes' entries
  in the catalogs.

  The five basic clauses are checked in the following
  order:

  1. From_clause: Check all relation names to make sure that
  they exist in the catalog. Also check that use of tuple or range 
  variables is correct. The From_clause is not decorated with any 
  info as it will be pruned from the tree when the operator tree 
  is constructed. A pointer, leftmost_rel, is set to the beginning 
  of the relation list. This pointer will be accessed when checking 
  the other clauses, i.e. any relations named in the other clauses 
  must be in the relation list of the From_clause, and any
  unqualified attributes used in the other clauses must appear in 
  one and only one of the relations in the relation list.

  2. Select_clause: check attributes using the leftmost_rel pointer 
  and decorate all attribute references. See below for specifics
  on what checks are made.

  3. Where_clause: check attributes using the leftmost_rel pointer 
  and decorate all attribute references. See below for specifics
  on what checks are made.

  4. Group_by_clause: check attributes using the leftmost_rel pointer 
  and decorate all attribute references. See below for specifics
  on what checks are made.

  3. Order_by_clause: check attributes using the leftmost_rel pointer 
  and decorate all attribute references. See below for specifics
  on what checks are made.


  ****************************************************************/

#include "typechk.h"

#include "parser.h"
#include "ii_systemcatalogs.h"
#include "ext_globals.h"
#include "hashing.h"
#include "compiler_tables.h"
#include "scan_routines.h" // for strcmp_ours

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define IS_JOIN 		-1
#define ERROR_TYPE 	-2



/* the error file. */
// extern FILE *f1; // removed by Navin Kabra: 9/18/93

/* internal functions */
static void check_from_clause(pp_NODPTR);
static void check_select_clause(pp_NODPTR);
static void check_where_clause(pp_NODPTR);
static void check_group_by_clause(pp_NODPTR);
static void check_order_by_clause(pp_NODPTR);
static void check_having_by_clause(pp_NODPTR);
static void check_and_list(pp_NODPTR ptr, bool aggs=false);
static void check_relop(pp_NODPTR, bool aggs=false);
static void check_relop_left(pp_NODPTR, bool aggs=false);
static void check_relop_right(pp_NODPTR, bool aggs=false);
static void check_join(pp_NODPTR);
static Attribute* find_attr_id(pp_NODPTR); 
static Attribute* find_attr_dot(pp_NODPTR); 
static int check_lit(Attribute *, int, pp_NODPTR);
static void check_aggr_int_float(Attribute *, pp_NODPTR);
bool isAggregate(pp_NODE_TYPE type);

// Added by Navin Kabra:
int err (int, char *);
int unique_var (pp_NODPTR);


static int    relation_count;		/* num of relations in from_clause */
static pp_NODPTR leftmost_rel;		/* keep ptr to relation list in 
					   from_clause for easy reference
					   from other clauses  */

#include <stdlib.h>
#include <string.h>
#include <malloc.h>

// #include <typechk_func_defs.h> // removed by Navin Kabra: 9/18/93

int pp_typechk(pp_NODPTR nod, bool aggs)
{
  if(nod != NULL)
    {
      switch(nod->type)
	{
	case nt_intlit :
	  break;
	case nt_floatlit :
	  break;
	case nt_strlit :
	  break;
	case nt_id :
	  break;
	case nt_query :
	  pp_typechk(nod->nodeleftchild);
	  break;

	case nt_query_stmt :
	  nod = nod->nodeleftchild;

	  leftmost_rel = nod->nodeleftchild;
	  check_from_clause(nod->nodeleftchild);
	  nod = nod->nodenext;

	  check_select_clause(nod->nodeleftchild);
	  nod = nod->nodenext;

	  check_where_clause(nod);
	  nod = nod->nodenext;

	  check_group_by_clause(nod);
	  nod = nod->nodenext;

	  check_having_by_clause(nod);
	  nod = nod->nodenext;
	  
	  check_order_by_clause(nod);
	  break;

	case nt_set_op :
	  break;
	case nt_intersect :
	  break;
	case nt_minus :
	  break;
	case nt_set_union :
	  break;
	case nt_group :
	  break;
	case nt_order :
	  break;
	case nt_all :
	  break;
	case nt_distinct :
	  break;
	case nt_star :
	  break;
	case nt_rel_name :
	  break;
	case nt_pred_list :
	  break;
	case nt_not :
	  break;
	case nt_and :
	  check_and_list(nod->nodeleftchild, aggs);
	  break;
	case nt_or :
	  pp_typechk(nod->nodeleftchild, aggs);
	  pp_typechk(nod->noderightchild, aggs);
	  break;
	case nt_between :
	  break;

	case nt_equal :
	case nt_not_equal :
	case nt_less :
	case nt_greater :
	case nt_less_eq :
	case nt_greater_eq :
	  check_relop(nod);
	  break;

	case nt_u_minus :
	  break;
	case nt_add_op :
	  break;
	case nt_sub_op :
	  break;
	case nt_mul_op :
	  break;
	case nt_div_op :
	  break;
	case nt_aggregate :
	  break;
	case nt_int_lit :
	  break;
	case nt_float_lit :
	  break;
	case nt_str_lit :
	  break;
	case nt_count :
	  break;
	case nt_avg :
	  break;
	case nt_max_nod :
	  break;
	case nt_min_nod :
	  break;
	case nt_sum :
	  break;
	case nt_dot :
	  break;
	case nt_asc :
	  break;
	case nt_dsc :
	  break;
	case nt_u_plus :
	  break;
	case nt_is :
	  break;
	case nt_null :
	  pp_typechk(nod->nodenext, aggs);
	  break;
	case nt_utility:
	  // this case added by Navin Kabra: 10/15/93
	  // no typechecking to be done...
	  // at least for now...
	  break;
	default:
	  ParserErr.SetState(SysErr::PBadNode);
	//  err(nod->pp_line_no,"unexpected node type in typechk.c main switch stmt");
	//  exit(-1);
	} /* end case */ 

    }/* end if */
  return 0;
}


static void check_where_clause(pp_NODPTR nod)
{
//  String       relation_name;
//  Plan_Relation     *rel  = NULL;

  if(nod->type == nt_null)
    {

/*
      if(relation_count != 1)
	{
	  // err(nod->pp_line_no, "where_clause is empty but from clause has more than one relation.");
	  ParserErr.SetState(SysErr::PEmptyWhere);

	  return;
	} 
*/

/* May 13, 1996 -- Luke Blanshard

   I commented this code out because it doesn't do anything.  I have no idea
   what it was supposed to do.

      if(leftmost_rel->type == nt_id)
	{
	  relation_name = leftmost_rel->nodeval.id_val->id_name;
	}
      else
	{
	  relation_name = leftmost_rel->nodeleftchild->nodeval.id_val->id_name;
	}
      rel = systemCatalogs.GetPlan_Relation(relation_name);
*/
    }

  /* else where_clause not null, type check it */
  pp_typechk(nod->nodeleftchild);
}

static void check_group_by_clause(pp_NODPTR nod)
{
  Attribute *attr;


  if(nod->type == nt_null)
    return;

  nod = nod->nodeleftchild;

  while(nod)
    {
      attr = NULL;

      switch(nod->type)
	{
	case nt_id:	/* unqualified attrib */
	  
	  attr = find_attr_id(nod);
	  break;
	  
	case nt_dot:
	  /* dot->nodeleftchild may be either relation name or tuple var */

	  attr = find_attr_dot(nod);
	  break;
	  
	default: 
	  ParserErr.SetState(SysErr::PBadNode);
	  // err(nod->pp_line_no,"in check_group_by_clause(), unexpected node type");
	  break;
	}

      /* set field_no, relation_no, pp_attr_desc, pp_rel_desc for the found attrib */
      nod->type_info = attr;

      nod = nod->nodenext;
    }
  
}

static void check_having_by_clause(pp_NODPTR nod)
{

    if (nod->type == nt_null)
	return;

    return;
    
    pp_typechk(nod->nodeleftchild, true); // aggregates are ok in having
   
//    if (nod->type != nt_null)
//	ParserErr.SetState(SysErr::PHaving);
    
    return;    
}


static void check_order_by_clause(pp_NODPTR nod)
{
  Attribute *attr;


  if(nod->type == nt_null)
    return;

  nod = nod->nodeleftchild;

  while(nod)		/* check each attrib in order_by clause
			   and set its field_no, relation_no, 
			   pp_attr_desc, and pp_rel_desc */
  {
      attr = NULL;
      
      switch(nod->type)
      {
	  /* each attrib is followed by an asc, dsc, or
	     null node */
	case nt_asc:
	case nt_dsc:
	case nt_null:
	  break;
	  
	case nt_id:	/* unqualified attrib */
	  
	  attr = find_attr_id(nod);
	  break;
	  
	case nt_dot:
	  /* dot->nodeleftchild may be either relation name or tuple var */

	  attr = find_attr_dot(nod);
	  break;
	  
	default: 
	  ParserErr.SetState(SysErr::PBadNode);
	  // err(nod->pp_line_no,"in check_order_by_clause(), unexpected node type");
	  break;
	}

      /* set field_no, relation_no, pp_attr_desc, pp_rel_desc for the found attrib */
      nod->type_info = attr;

      nod = nod->nodenext;
    }
  
}



/******************    check_and_list    ***************** 
  AND dominates a list of ops which are doubly linked.
  nod is ptr to leftmost child of AND, i.e., the first
  item in the doubly linked list.
  */
static void  check_and_list(pp_NODPTR nod, bool aggs)
{
  while(nod)
    {
      switch(nod->type)
	{
	case nt_equal:
	case nt_not_equal:
	case nt_less:
	case nt_greater:
	case nt_less_eq:
	case nt_greater_eq:
	  check_relop(nod, aggs);
	  break;
	case nt_or:
	  pp_typechk(nod, aggs);
	  break;
	  /* this will be eliminated. */
	case nt_and:
	  pp_typechk(nod, aggs);
	  break;
	default:
	  ParserErr.SetState(SysErr::PBadNode);
	  // err(nod->pp_line_no,"unexpected node type in check_and_list().");
	}
      nod = nod->nodenext;
    }
}

bool isAggregate(pp_NODE_TYPE type)
{
    bool isAgg;
    
    switch(type)
    {
      case nt_avg:
      case nt_min_nod:
      case nt_max_nod:
      case nt_sum:
      case nt_count:
	isAgg = true;
	break;
      default:
	isAgg = false;
	break;
    }
    return isAgg;	
}


/*******************     check_relop     **********************  
  Relop node dominates ids on both left and right (a join)
  or an id on the left and a literal on the right (a select).
  For a select the relation_no of the relop node is set to
  the relation_no of its left child.
  For a join the relation_no of the relop node is set to IS_JOIN
  (-1) to signal the join.
  */
static void check_relop(pp_NODPTR nod, bool aggs)
{
  Attribute *attr;
  int		  type_ok = FALSE;	/* set to TRUE if ok */

  /* set type_info for left and right children */
  check_relop_left(nod->nodeleftchild, aggs);
  check_relop_right(nod->noderightchild, aggs);

  /* if right child type is id or dot, this 
     is a join, set type_info->relation_no
     to IS_JOIN (= -1) to signal that this
     is a join */

  if((nod->noderightchild->type == nt_id) ||
     (nod->noderightchild->type == nt_dot) ||
     isAggregate(nod->noderightchild->type))
    {
      /* type match left and right attrib types of join */
      check_join(nod);

      return;
    }

  /* null right child signals 'attrib is (not) null' */
  if((nod->noderightchild->type == nt_null) &&
     ((nod->type == nt_equal) || (nod->type == nt_not_equal)) )
    {
      double temp_float = 0.0;

      /* get type of attrib on left */
      attr = nod->nodeleftchild->type_info;

      /* set right child to Zero (0 for int, 0.0 for float, "" for string) */
      if(attr)
	switch(attr->Type())
	  {
	  case attrInteger:
	    nod->noderightchild->type    = nt_int_lit;
	    nod->noderightchild->nodeval.int_val = 0;
	    break;

	  case attrReal:
	    nod->noderightchild->type            = nt_float_lit;
	    nod->noderightchild->nodeval.float_val = (double *) &temp_float;
	    break;

	  case attrString:
	    nod->noderightchild->type            = nt_str_lit;
	    nod->noderightchild->nodeval.str_val = (char *)"";
	    break;

	  default:
	    ParserErr.SetState(SysErr::PNullUseError);
	    // err(nod->pp_line_no, "NULL or NOT NULL used for non basic type");
	  }


      return;
    }

  /* else, this is a select, right child should be
     a lit expr so set nod->type_info to that of 
     the select attrib on the left. 
     At this point either the id node on left or
     the dot node on left has the attrib info. 
     If the node is a dot node, its right child (id)
     also has same attrib info. */
  
  if((nod->nodeleftchild->type == nt_id) ||
     (nod->nodeleftchild->type == nt_dot) ||
     isAggregate(nod->nodeleftchild->type)) 
    {

      attr = nod->nodeleftchild->type_info;

      /* Check type of attrib on left against lit on right */
      if(attr)
	type_ok = check_lit(attr, nod->noderightchild->type, nod);

      /* give the select (Equal) node a relation_no */
      if(type_ok)
	{
	  nod->type_info = new Attribute(*attr);
	}
      else	/* set error val */
	{
          nod->type_info = NULL;
	}

      return;
    }
  ParserErr.SetState(SysErr::PBadNode);
  // err(nod->pp_line_no,"unexpected node type in check_relop()");
}



/*****************     check_relop_right     **************
  Check the right child under a relop
  */
static void check_relop_right(pp_NODPTR right, bool aggs)
{
  Attribute *attr = NULL;

  switch(right->type)
    {
    case nt_id:	/* unqualified attrib */
      
      attr = find_attr_id(right);

      if(! attr)
	return;

      right->type_info = attr;
      return;
      
    case nt_dot: // dot->nodeleftchild may be either relation name or tuple var

      attr = find_attr_dot(right);
      
      if(!attr)
	return;
      
      right->type_info = attr;
      right->noderightchild->type_info = new Attribute(*attr);
      return;
      
    case  nt_null:
      /* null as right child of 'equal' or 'not_equal' is parse
	 representation of 'attrib is (not) null'               */
      return;

      /* check lits above from relop nod */
    case  nt_intlit:
    case  nt_floatlit:
    case  nt_strlit:
      return;

    case nt_avg:
    case nt_min_nod:
    case nt_max_nod:
    case nt_sum:
      if (!aggs)
	  ParserErr.SetState(SysErr::PBadNode);
      else if(right->noderightchild->type == nt_id)    // unqualified attrib 
      {
	  attr = find_attr_id(right->noderightchild);
	  right->noderightchild->type_info=attr;
      } 
      else if(right->noderightchild->type == nt_dot)
      {
	  attr = find_attr_dot(right->noderightchild);
	  right->noderightchild->type_info=attr;
      }
      else
      {
	  ParserErr.SetState(SysErr::PBadNode);
      }
      if(attr)	/* check attrib type, want int or float */
      {
	  check_aggr_int_float(attr, right);
      }
      return;
      
    case nt_count:
      if (!aggs)
	  ParserErr.SetState(SysErr::PBadNode);      
      else if(right->nodeleftchild->type == nt_star)
      {
	  /* attr kept NULL, so nod->type_info will be NULL too */
	  break;
      }
      if(right->noderightchild->type == nt_id)    // unqualified attrib 
      {
	  right->noderightchild->type_info = find_attr_id(right->noderightchild);
      }
      else if(right->noderightchild->type == nt_dot)
      {
	  right->noderightchild->type_info = find_attr_dot(right->noderightchild);
      }
      break;
	  
    default: 
      ParserErr.SetState(SysErr::PBadNode);
      //err(right->pp_line_no,"in check_relop_right(), unexpected node type");
      return;

    }
}



/*****************     check_relop_left     **************
  Check the left child under a relop
  */
static void check_relop_left(pp_NODPTR left, bool aggs)
{
  Attribute *attr = NULL;

  if(left->type == nt_id)	/* unqualified attrib */
    {
      attr = find_attr_id(left);
    }
  else if(left->type == nt_dot)
    { /* dot->nodeleftchild may be either relation name or tuple var */

      attr = find_attr_dot(left);

      /* set field_no for the found attrib on id */
      if ( attr )
          left->noderightchild->type_info = new Attribute(*attr);
    }
  else if (aggs && isAggregate(left->type))
  {
      switch (left->type)
      {
	case nt_avg:
	case nt_min_nod:
	case nt_max_nod:
	case nt_sum:
	  if(left->noderightchild->type == nt_id)    // unqualified attrib 
	  {
	      attr = find_attr_id(left->noderightchild);
	      left->noderightchild->type_info=attr;
	  } 
	  else if(left->noderightchild->type == nt_dot)
	  {
	      attr = find_attr_dot(left->noderightchild);
	      left->noderightchild->type_info=attr;
	  }
	  else
	  {
	      ParserErr.SetState(SysErr::PBadNode);
	  }
	  if(attr)	/* check attrib type, want int or float */
	  {
	      check_aggr_int_float(attr, left);
	  }
	  return;
	  
	case nt_count:
	  if (!aggs)
	      ParserErr.SetState(SysErr::PBadNode);      
	  else if(left->nodeleftchild->type == nt_star)
	  {
	      /* attr kept NULL, so left->type_info will be NULL too */
	      break;
	  }
	  if(left->noderightchild->type == nt_id)    // unqualified attrib 
	  {
	      attr = find_attr_id(left->noderightchild);
	  }
	  else if(left->noderightchild->type == nt_dot)
	  {
	      attr = find_attr_dot(left->noderightchild);
	  }
	  break;
	  
	default: 
	  ParserErr.SetState(SysErr::PBadNode);
	  //err(right->pp_line_no,"in check_relop_right(), unexpected node type");
	  return; 
	  
      }      
  }
  else 
  {
      ParserErr.SetState(SysErr::PBadNode);
      // err(left->pp_line_no,"in check_relop_left(), unexpected node type");
      return;
  }

  /* set field_no for the found attrib on id or dot node */
  left->type_info = attr;
}



/******************    check_join     ******************* 
  type check the left and right attribs
  of the join to see if they are type
  compatible.
  The relop node has either an id or
  a dot node on the left and the right.
  The dot and id nodes have ptrs to
  attr cat in their type_info structs.
  */
static void check_join(pp_NODPTR relop)
{
  if((relop->nodeleftchild->type_info) &&
     (relop->noderightchild->type_info))
    /* if left or right is NULL, error already reported above */
    {
      if(relop->nodeleftchild->type_info->Type() !=
	 relop->noderightchild->type_info->Type())
	ParserErr.SetState(SysErr::PTypeMismatch);
//	err(relop->pp_line_no, "type mismatch on left and right of join");
    }
}



/*****************     check_select_clause     **************** 
  Make sure that all relations are given in the from_clause
  and that all attribs are from relations in the from_clause
  For all attribs, set their field_no

  nod points to head of select list, type is all, distinct, null, or star
  */
static void check_select_clause(pp_NODPTR nod)
{
    Attribute *attr;
    
    nod = nod->nodenext;     // now type is star, id, or dot or an aggregate
    
    if(nod->type == nt_star) // No decoration, entire select_clause
	return;              // will be pruned from
			     // operator tree 
  
  while(nod)		   // check each attrib in select clause
  {                        // and set its field_no, relation_no, 
      attr = NULL;  	   // pp_attr_desc, and pp_rel_desc

      switch(nod->type)
      {
	case nt_id:	/* unqualified attrib */
	  
	  attr = find_attr_id(nod);
	  break;
	  
	case nt_dot:
	  /* dot->nodeleftchild may be either relation name or tuple var */

	  attr = find_attr_dot(nod);
	  break;
	  
	  
	  /* for avg,min,max,sum: expr must be either int or float */
	  /* The aggregate node itself is decorated with type info
	     not the id or dot node below it */
	case nt_avg:
	case nt_min_nod:
	case nt_max_nod:
	case nt_sum:

	  if(nod->noderightchild->type == nt_id)    // unqualified attrib 
	  {
	      attr = find_attr_id(nod->noderightchild);
              if ( attr )
                  nod->noderightchild->type_info = new Attribute(*attr);
	  }
	  else if(nod->noderightchild->type == nt_dot)
	  {
	      attr = find_attr_dot(nod->noderightchild);
              if ( attr )
                  nod->noderightchild->type_info = new Attribute(*attr);
	  }
	  else
	  {
	      ParserErr.SetState(SysErr::PBadNode);
	      // err(nod->pp_line_no, "unexpected node type in aggregate of select clause");
	  }
	  if(attr)	/* check attrib type, want int or float */
	  {
	      check_aggr_int_float(attr, nod);
	  }
	  break;
	  
	case nt_count:
	  if(nod->nodeleftchild->type == nt_star)
	  {
	      /* attr kept NULL, so nod->type_info will be NULL too */
	      break;
	  }
	  if(nod->noderightchild->type == nt_id)    // unqualified attrib 
	  {
	      attr = find_attr_id(nod->noderightchild);
	  }
	  else if(nod->noderightchild->type == nt_dot)
	  {
	      attr = find_attr_dot(nod->noderightchild);
	  }
	  break;
	  
	default: 
	  ParserErr.SetState(SysErr::PBadNode);
	  // err(nod->pp_line_no,"in check_select_clause(), unexpected node type");
	  break;
	}
      
      /* set field_no, relation_no, pp_attr_desc, pp_rel_desc for the found attrib */
      /* for "count(*) attr_info is NULL */
      nod->type_info = attr;
      
      nod = nod->nodenext;
    }
}



/******************     find_attr_dot    ********************* 
  Given a dot node which has rel as left child and attrib as right child,
  find and return its attr in the catalog.

  The left child is either the name of a relation, deposit.amount
  or a tuple var, S.amount
  */
static Attribute* find_attr_dot(pp_NODPTR dot_nod)
{
  char          *rel_name;	/* relation or var on left of dot */
  char	  *attr_name;	        /* attrib name on right of dot */ 
  char	  *from_rel = 0;	/* relation in From clause */
  pp_NODPTR         rel_node;
  Attribute        *a = NULL;
  
  rel_name =  dot_nod->nodeleftchild->nodeval.id_val->id_name;
  attr_name = dot_nod->noderightchild->nodeval.id_val->id_name;

  /* first assume left child is a rel name and look for match in
     rel list of From clause */

  for (rel_node = leftmost_rel; rel_node; rel_node = rel_node->nodenext)
    {
      if(rel_node->type == nt_id)
	from_rel = rel_node->nodeval.id_val->id_name;
      else	/* type == nt_rel_name */
	from_rel = rel_node->nodeleftchild->nodeval.id_val->id_name;

      if(strcmp_ours(from_rel, rel_name) == 0)
	{
	  /* rel on left of dot pp_OK, so fetch attr tuple */
	  a = systemCatalogs->GetAttribute(rel_name, attr_name);

	  if(!a)
	    ParserErr.SetState(SysErr::PBadAttr);
	  //  err(dot_nod->pp_line_no, "use of nonexistent dot-qualified attribute");
          return a;
	}

    }

  /* if not found as a rel name, assume that left child is a
     tuple var and look for a match */
  
  for (rel_node = leftmost_rel; rel_node; rel_node = rel_node->nodenext)
    {
      if(rel_node->type == nt_rel_name)
	from_rel = rel_node->noderightchild->nodeval.id_val->id_name;

      if(strcmp_ours(from_rel, rel_name) == 0)
	{
	  /* tuple var on left of dot matches, so fetch attr tuple */
	  from_rel =  rel_node->nodeleftchild->nodeval.id_val->id_name;
	  a = systemCatalogs->GetAttribute(from_rel, attr_name);

	  if(!a)
	    ParserErr.SetState(SysErr::PBadAttr);
	   // err(dot_nod->pp_line_no, "use of nonexistent attribute with tuple var");
	  return a;
	}
    }

  /* if reach here, relation or tuple var was not found in From clause's relation list */
  ParserErr.SetState(SysErr::PBadAttr);
  // err(dot_nod->pp_line_no, "use of nonexistent attribute");
  return NULL;
}



/******************     find_attr_id     ************************ 
  Find the attrib tuple in the catalog given an id node.
  Unqualified attrib names used in select, where, etc should be
  unique among the relations given in the from clause.
  */
static Attribute* find_attr_id(pp_NODPTR nod)
{
  char          *rel_name;
  char	        *attr_name;
  pp_NODPTR         rel_node;	/* relation list of from_clause */
  Attribute        *result = NULL, *a = NULL;
  int            found_count = 0;    /* if more than 1 attr found, ambiguous */


  attr_name = nod->nodeval.id_val->id_name;

  /* traverse rel list of From clause, look for rel with this attribute */
  for (rel_node = leftmost_rel; rel_node; rel_node = rel_node->nodenext)
    {
      if(rel_node->type == nt_id)
	rel_name = rel_node->nodeval.id_val->id_name;
      else 	/* type == nt_rel_name */
	rel_name = rel_node->nodeleftchild->nodeval.id_val->id_name;

      a = systemCatalogs->GetAttribute(rel_name, attr_name);

      if(a)	/* attr found for relation_name */
	{
	  found_count++;
	  
	  if(found_count > 1)
	    {
              delete a;
              delete result;
	      ParserErr.SetState(SysErr::PBadAttr);
	      // err(nod->pp_line_no, "use of ambiguous attribute");
	      return NULL;
	    }
          result = a;
	}
    }

  if(found_count == 0)
    {
      ParserErr.SetState(SysErr::PBadAttr);
      // err(nod->pp_line_no, "use of nonexistent attribute");
      return NULL;
    }

  return result;	/* unique attribute found */
}



/***************     check_from_clause     *****************
  Traverse relation list of the from_clause
  checking to see that each relation in the
  list is in the relation catalog.
  Each node is either an id, which is simply a 
  relation name, or is a rel_name dominating 
  two ids, e.g. deposit S
  nod is ptr to first relation in list and
  should never enter as NULL
  The catalog is accessed to verify the existence
  of the relations.
  The global leftmost_rel is set to the first relation
  in the list.
  The global relation_count records how many relations
  are in the relation list.
  */
static void check_from_clause(pp_NODPTR nod)
{
  relation_count = 0;

  while(nod)
    {
      if(nod->type == nt_id)
	{
	  /* get rel descriptor */
	  if ( !systemCatalogs->RelationExists( nod->nodeval.id_val->id_name ) )
	    {
	      ParserErr.SetState(SysErr::PBadRel);
	      // err(nod->pp_line_no, "non-existent relation in From clause");
	    }
	}
      else if(nod->type == nt_rel_name)	/* should be nt_rel_name */
	{
	  
	  /* check tuple var for uniqueness  */
	  unique_var(nod);

          if ( !systemCatalogs->RelationExists( nod->nodeleftchild->nodeval.id_val->id_name ) )
	    {  
	      ParserErr.SetState(SysErr::PBadRel);
	      // err(nod->pp_line_no, "non-existent relation in From clause");
	    }
	}
      else
	ParserErr.SetState(SysErr::PBadRel);
//	err(nod->pp_line_no, "in typechk.c, id or rel_name expected");
      relation_count++;
      nod = nod->nodenext;
    }
}

/*******************     unique_var     ****************** 
  Check that a tuple var is not used more than once,
  i.e., deposit T, borrow S is ok
  deposit T, borrow T is an error
  
  var is a ptr to rel_name whose right child is a tuple var
  */
int unique_var(pp_NODPTR var)
{
  pp_NODPTR check = var->nodenext;

  while( check )
    {
      if(check->type == nt_rel_name)
	{
	  if(strcmp_ours(check->noderightchild->nodeval.id_val->id_name,
			    var->noderightchild->nodeval.id_val->id_name) == 0)
	    {
	      ParserErr.SetState(SysErr::PNonUniqueVar);
	      // err(check->noderightchild->pp_line_no, "non unique tuple var");
	      
	    }
	}
      check = check->nodenext;
    }

  /* check that tuple var not identical with name of existing relation */
  if ( systemCatalogs->RelationExists( var->noderightchild->nodeval.id_val->id_name ) )
    ParserErr.SetState(SysErr::PMissingComma);
    // err(var->pp_line_no, "missing comma or tuple var same as an existing relation name");
  
  return 0;
}




/******************     err     ********************
  print an error message and set global error 
  flag to 0
  */
int err(int line, char *msg)
{
  if(line != 0)
    //	fprintf(f1,"line %d : error : %s\n",line, msg); // Navin Kabra: 9/18/93
    fprintf(stderr,"error - %s\tline %d : \n", msg, line);
  else
    //	fprintf(f1,"join use  error : %s\n", msg); // Navin Kabra: 9/18/93
    fprintf(stderr,"error - join use error : %s\n", msg);

//  no_error = 0;
  ParserErr.SetState(SysErr::PParseError);
  return 0;
}


/**************************   !!!!!!!!!!!!!!!!!      ****************************************
  ZZ everything above is indepentdent of catalogs and routines in catalog.c
  (as long as catalog.c routines return the desired info). (Except for one place marked 'ZZ')

  The routines below assume that type info is stored as a char in the catalogs
  so some revisions may be necessary
  **********************************/

/**************     check_aggr_int_float    ***************
  All aggregates except COUNT must be either int or float

  ZZ Note: this check is dependent on how types are represented in
  the catalogs, in current catalogs, int is char INTEGER and 
  float is char REAL

  */
static void check_aggr_int_float(Attribute *attr, pp_NODPTR /*nod*/)
{
  if((attr->Type() != attrInteger) && (attr->Type() != attrReal))
    ParserErr.SetState(SysErr::PIntOrFloatForAgg);
    // err(nod->pp_line_no, "need int or float for aggregate");

}


/******************     check_lit     ******************** 
  Check that an attribute has the same type
  as the lit that it is compared to

  ZZ Note: this function depends on the implemation of
  attribute types within the catalogs, in our current
  catalogs, type (char, int, float) is coded as a char
  TEXT, INTEGER, REAL
  */
static int check_lit(Attribute *attr, int lit_type, pp_NODPTR /*nod*/)
{
  AttrType attrtype;

  attrtype = attr->Type();

  switch(lit_type)
    {
    case nt_intlit:
      if(attrtype != attrInteger && attrtype != attrReal)
	{
	  ParserErr.SetState(SysErr::PTypeMismatch);
	  // err(nod->pp_line_no,"type mismatch, lit is int, attrib is not");
	  return FALSE;
	}
      return TRUE;

    case nt_floatlit:
      if(attrtype != attrReal)
	{
	  ParserErr.SetState(SysErr::PTypeMismatch);
	  // err(nod->pp_line_no,"type mismatch, lit is float, attrib is not");
	  return FALSE;
	}
      return TRUE;

    case nt_strlit:
      if(attrtype != attrString)
	{
	  ParserErr.SetState(SysErr::PTypeMismatch);
	 // err(nod->pp_line_no,
	    //  "literal is string, attribute is not");
	  return FALSE;
	}
      return TRUE;

    default:
      ParserErr.SetState(SysErr::PBadNode);
      // err(nod->pp_line_no,"default reached in check_lit()");
      return FALSE;
    }
}

