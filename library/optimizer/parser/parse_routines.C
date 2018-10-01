/*******************************************************************************
*                                                                              *
*      SQL Front-End Interactive System for Gamma & Exodus                     *
*                                                                              *
*                  (c) Dec. 1992 - Manuvir Das, University of Wisconsin        *
*                                                                              *
*      Permission is granted to copy all or part of this code, provided this   *
*      header is present in its entirety in the copied version of the code.    *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
*                                                                              *
*      parse_routines.c : utility routines for the SQL parser                  *
*                                                                              *
*******************************************************************************/

#include "query.h"
#include <stdio.h>
#include <stdlib.h>
#include <ext_sym_tabs.h>
#include <ext_token.h>
// #include <ext_types.h>
#include <malloc.h>
#include <parse_func_defs.h>
#include <real_parse_stack.h>

extern int line_no, col_no;
NODPTR Entry;                // a hack by SCH

/* Create a new node in the syntax tree and return pointer to it. */

int makenode(NODPTR *nod,NODE_TYPE type1)
{

/* Locals. */

/* Allocate space for node. */

	(*nod) = (NODPTR) malloc ( sizeof (AST_NODE) );

/* Set its node type to type. */

	(*nod)->type = type1;
	(*nod)->visited = 0;

/* Set the node value. Use a switch. */

	switch ( type1 ) {

	case intlit :
		{
		(*nod)->nodeval.int_val = current_tok->actual_value.int_val;
		}
		break;

	case floatlit :
		{
		(*nod)->nodeval.float_val = (double *)(current_tok->actual_value.float_val);
		}
		break;

	case strlit :
		{
		(*nod)->nodeval.str_val = (char *)(current_tok->actual_value.str_val);
		}
		break;

	case id :
		{
		(*nod)->nodeval.id_val = current_tok->actual_value.id_val;
		}
		break;

	default : 
		{
		(*nod)->nodeval.int_val = 0;
		}

	}
	(*nod)->type_info = NULL;
	(*nod)->line_no = line_no;
	(*nod)->col_no = col_no;

	(*nod)->nodeleftchild = NULL ;
	(*nod)->noderightchild = NULL ;
	(*nod)->nodeprev = NULL ;
	(*nod)->nodenext = NULL ;

	return(line_no);

}


void freenode( void* node )
{
    NODPTR n = (NODPTR)node;
    if ( n->nodeleftchild )
        freenode( n->nodeleftchild );
    if ( n->noderightchild )
        freenode( n->noderightchild );
    delete n->type_info;
    free(n);
}


/* Give birth. ( Haha ). */

void link_to_parent(NODPTR child,NODPTR parent)
{
	child->nodeparent = parent;
}

/* Is this adoption or what ? */

void link_to_left_child(NODPTR parent,NODPTR left_child)
{
	parent->nodeleftchild = left_child;
}

/* This parent wants to adopt two kids. Unusual. */

void link_to_right_child(NODPTR parent,NODPTR right_child)
{
	parent->noderightchild = right_child;
}

/* The sibling stuff. */

void link_to_next(NODPTR cur_node,NODPTR next_node)
{
	cur_node->nodenext = next_node;
}

void link_siblings(NODPTR left,NODPTR right)
{
	left->nodenext = right;
	right->nodeprev = left;
}

/* Younger brother thinking of bhaiya ... */

void link_to_prev(NODPTR cur_node,NODPTR prev_node)
{
	cur_node->nodeprev = prev_node;
}

void link_all_to_parent(NODPTR parent,NODPTR right,NODPTR *left)
{
    NODPTR nod1 = right, nod2 = 0;
    while ( nod1 )
      {
        link_to_parent(nod1,parent);
        nod2 = nod1;
        nod1 = nod1->nodeprev;
      }
    *left = nod2; 
}
