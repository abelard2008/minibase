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
*      code_generator.c : conversion of an optimized plan to a query packet    *
*                                                                              *
*******************************************************************************/

/*

	Parse Stack Definitions for SQL Translator

	CS764 Course Project

		Ted Hill
		Manuvir Das			 */

#ifndef PARSE_STACK_H
 // added this
#define PARSE_STACK_H 
 // Navin Kabra: 6/16/93.

#include "hashing.h"
#include "query.h"
#include <mr_syserr.h>

/* The node type. */
typedef int  pp_NODE_TYPE;

/* Node type possibilities. For query statements only. */

#define	nt_intlit		1
#define	nt_floatlit	2
#define	nt_strlit		3
#define	nt_id		4
#define	nt_query		5
#define	nt_query_stmt	8
#define	nt_set_op		9
#define	nt_intersect	10
#define	nt_minus		11
#define	nt_set_union	12 
#define	nt_select		13
#define	nt_from		14
#define	nt_where		15
#define	nt_group		16
#define	nt_order		17
#define	nt_all		18
#define	nt_distinct	19
#define	nt_star		20



/* Not used yet? */

#define	nt_col_name	21
#define nt_select_list	22
#define	nt_reln_list	23
#define	nt_col_list	50
#define	nt_order_list	51
#define	nt_col_number	52
#define	nt_set_order_list	56
#define	nt_column		57

#define	nt_rel_name	24
#define	nt_pred_list	25
#define	nt_not		26
#define	nt_and		27
#define	nt_or		28
#define	nt_between		29
#define	nt_equal		30
#define	nt_not_equal	31
#define	nt_less		32
#define	nt_greater		33
#define	nt_less_eq		34
#define	nt_greater_eq	35
#define	nt_u_minus		36
#define	nt_add_op		37
#define	nt_sub_op		38
#define	nt_mul_op		39
#define	nt_div_op		40
#define	nt_aggregate	41
#define	nt_int_lit		42
#define	nt_float_lit	43
#define	nt_str_lit		44
#define	nt_count		45
#define	nt_avg		46
#define	nt_max_nod		47
#define	nt_min_nod		48
#define	nt_sum		49
#define	nt_dot		53
#define	nt_asc		54
#define	nt_dsc		55
#define	nt_u_plus		65
#define	nt_is		66
#define	nt_null		67
#define	nt_create_db	68
#define	nt_drop_db		69
#define	nt_create_table	70
#define	nt_drop_table	71
#define	nt_drop_index	72
#define	nt_create_index	73
#define	nt_integer_typ	74
#define	nt_character_typ	75
#define	nt_float_typ	76
#define	nt_utility		77
#define	nt_uniq		78
#define	nt_open_db		79
#define	nt_close_db	80
#define	nt_insert		81
#define	nt_key_nod		82
#define	nt_load_table	83
// added by mgl
#define nt_having       84


// Added for transactions - RR

#define nt_xbegin       86
#define nt_xcommit      87
#define nt_xabort       88
#define nt_xprint       89

// End of additions for transactions - RR

// there is something wierd here with having clause.  don't worry about it now
// we'll get to it when the time is right.

/* For utility operations. To be handled later. */

/*
#define	utility		6
#define	update		7
#define	idx_attr_list	58
#define	data_type	59
#define	char_type	60
#define	int_type	61
#define	float_type	62
#define	data_type_list	63
#define	value_list	64

#define	any		90
*/

/* Union for nodeval. */

typedef union pp_lit_val {
	int int_val;
	char *str_val;
	pp_ID_ENTRY *id_val;
	double *float_val;
} pp_literal_val;

#define pp_LIT_VAL_DEFINED

/* A node in the syntax tree. Also a node that the stack type
   points to. */

typedef struct pp_ast_node {
	pp_NODE_TYPE type;
	struct pp_ast_node *nodenext, *nodeprev;
	struct pp_ast_node *nodeparent;
	struct pp_ast_node *nodeleftchild, *noderightchild; 
	Attribute *type_info;
	pp_literal_val  nodeval; /* ptr to symtab for ids, value for literals etc */
	int pp_line_no;
	int pp_col_no;
	} pp_AST_NODE;

/* The yacc stack type. */

typedef pp_AST_NODE *pp_NODPTR; 

/* Can't figure out what these do, but ... */
/* Oh ya, root node of AST. */

extern pp_NODPTR Entry;
extern int yyparse (void);

extern SysErr ParserErr;

#endif



