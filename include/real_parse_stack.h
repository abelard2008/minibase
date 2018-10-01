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

/* The node type. */

#ifndef _REAL_PARSE_STACK_H
#define _REAL_PARSE_STACK_H

typedef int  NODE_TYPE;

/* Node type possibilities. For query statements only. */

#define	intlit		1
#define	floatlit	2
#define	strlit		3
#define	id		4
#define	query		5
#define	query_stmt	8
#define	set_op		9
#define	intersect	10
#define	minus		11
#define	set_union	12
#define	select		13
#define	from		14
#define	where		15
#define	group		16
#define	order		17
#define	all		18
#define	distinct	19
#define	star		20


#define	col_name	21
#define select_list	22
#define	reln_list	23
#define	col_list	50
#define	order_list	51
#define	col_number	52
#define	set_order_list	56
#define	column		57


#define	rel_name	24
#define	pred_list	25
#define	not		26
#define	and		27
#define	or		28
#define	between		29
#define	equal		30
#define	not_equal	31
#define	less		32
#define	greater		33
#define	less_eq		34
#define	greater_eq	35
#define	u_minus		36
#define	add_op		37
#define	sub_op		38
#define	mul_op		39
#define	div_op		40
#define	aggregate	41
#define	int_lit		42
#define	float_lit	43
#define	str_lit		44
#define	count		45
#define	avg		46
#define	max_nod		47
#define	min_nod		48
#define	sum		49
#define	dot		53
#define	asc		54
#define	dsc		55
#define	u_plus		65
#define	is		66
#define	null		67
#define	create_db	68
#define	drop_db		69
#define	create_table	70
#define	drop_table	71
#define	drop_index	72
#define	create_index	73
#define	integer_typ	74
#define	character_typ	75
#define	float_typ	76
#define	utility		77
#define	uniq		78
#define	open_db		79
#define	close_db	80
#define	insert		81
#define	key_nod		82
#define	load_table	83
/* changed from 65 to 84 (already a 65!) -- MGL */
#define having		84
/* changed from 83 to 85 (already a 83!) -- RR */
#define deletefrom	85
/* added to add the concept of a transaction */
#define xbegin		86
#define xcommit		87
#define xabort		88
#define xprint		89


/* For utility operations. To be handled later. */


#define	update		7
#define	idx_attr_list	58
#define	data_type	59
#define	char_type	60
#define	int_type	61
#define	float_type	62
#define	data_type_list	63
#define	value_list	64


/* sub query stuff*/
#define any 66
#define notin  68
#define notmatch 69
#define in 70
#define match 71
#define exists 72
#define assignment 73

#define	dummy_as	100
#define	not_exists	101

#define TEMPVAR 73
#define TEMPRULE 74

/* Union for nodeval. */

typedef union lit_val {
	int int_val;
	char *str_val;
	ID_ENTRY *id_val;
	double *float_val;
} literal_val;

#define LIT_VAL_DEFINED

/* A node in the syntax tree. Also a node that the stack type
   points to. */

typedef struct ast_node {
	NODE_TYPE type;
	struct ast_node *nodenext, *nodeprev;
	struct ast_node *nodeparent;
	struct ast_node *nodeleftchild, *noderightchild; 
	class Attribute *type_info;
	literal_val  nodeval; /* ptr to symtab for ids, value for literals etc */
	int line_no;
	int col_no;
	int visited;
	} AST_NODE;

/* The yacc stack type. */

typedef AST_NODE *NODPTR; 

/* Can't figure out what these do, but ... */
/* Oh ya, root node of AST. */

extern NODPTR Entry;

#endif
