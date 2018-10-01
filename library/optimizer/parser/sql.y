/*******************************************************************************
*                                                                              *
*      SQL Front-End Interactive System for Gamma & Exodus                     *
*                                                                              *
*                  (c) Dec. 1992 - Manuvir Das, University of Wisconsin        *
*                                                                              *
*      Permission is granted to copy all or part of this code, provided this   *
*      header is present in its entirety in the copied version of the code.    *
* 
* bug: currently the grammar doesn't support queries of the form:
*            select * from emp where 23 > emp.dno;  (michaell@cs)
*                                                                             *
*      Credits : Edward Hill                                                   *
* $Id: sql.y,v 1.3 1996/07/07 20:26:49 ramamurt Exp $                 *         
* $Log: sql.y,v $
* Revision 1.3  1996/07/07 20:26:49  ramamurt
* Version with ext_sys_defs fixed, heapfiles and log bug fixed, new interface for startup - and with recovery partially working in a shared-mem environment. Xaction abort works. Also modified concept of transaction. One process can start and commit many transactions but interleaved xactions are still not allowed
*
* Revision 1.2  1996/04/21 15:19:41  luke
* Revamped space management; moved to gcc v2.7.1
*
* Revision 1.1  1996/02/11 04:26:57  luke
* Initial check-in of minibase
*
* Revision 1.1.1.1  1996/02/05 19:47:38  luke
* Initial import of Minibase library into CVS.
*
 * Revision 1.4  1995/06/19  19:37:22  michaell
 * making sure that parser returns orderby properly
 *
 * Revision 1.3  1995/04/25  18:50:39  michaell
 * *** empty log message ***
 *                                                              *
 * Revision 1.2  1995/04/06  20:58:39  michaell
 * michaell: added a clause to the predicate-list to fix valid sql that
 *           wasn't accepted.
 *                              
*******************************************************************************/

/*******************************************************************************
*                                                                              *
*      sql.y : yacc generated bottom-up parser for SQL                         *
*                                                                              *
*******************************************************************************/

%{
#include <stdio.h>
#include <ext_globals.h>
#include <ext_sym_tabs.h>
#include <real_parse_stack.h>
#include <parser_func_defs.h>
#include <mr_syserr.h>

#define YYSTYPE NODPTR

SysErr ParserErr;
extern int yylineno;
%}

%token INTEGER 		/* these for literals */
%token FLOAT 
%token STRING 

%token INT_TYPE		/* these for column types e.g. "age INT" */
%token FLOAT_TYPE 	/* "salary FLOAT" 			 */
%token CHAR_TYPE	/* "emp_name CHAR(20)" 			 */

%token IDENT

%token CREATE 
%token DROP
%token LOAD
%token OPEN 
%token CLOSE 
%token DB
%token TABLE
%token UNIQUE 
%token INDEX 
%token ON 
%token DELETE
%token INSERT
%token INTO
%token VALUES
%token UPDATE
%token SET
%token KEY

%token SELECT
%token FROM
%token WHERE
%token GROUP
%token ORDER
%token HAVING
%token BY

%token AS
%token ASC
%token DESC

%token XBEGIN
%token XABORT
%token XCOMMIT
%token XPRINT

%token INTERSECT
%token MINUS
%token UNION

%token COUNT
%token AVG
%token MAX
%token MIN
%token SUM 
%token ALL
%token STAR
%token DISTINCT

%token NOT
%token NUL 
%token AND
%token OR
%token IS
%token BETWEEN 

%token EQUAL 
%token NOT_EQUAL 
%token LESS 
%token GREATER
%token LESS_EQUAL
%token GREATER_EQUAL

%token L_PAREN
%token R_PAREN

%token COMMA
%token DOT

%token ANY
%token IN
%token MATCH
%token EXISTS

%left ADD
%left SUB
%left STAR
%left DIV
%left UNARY_ADD

%%
/* An SQL command can be a query, update or utility */
command:	  utility
	{ 
		makenode(&$$,utility);
                link_to_left_child($$,$1);
                link_to_right_child($$,$1);
                link_to_parent($1,$$);
		Entry = $$;
	}
		| update
	{ 
		makenode(&$$,utility);
                link_to_left_child($$,$1);
                link_to_right_child($$,$1);
                link_to_parent($1,$$);
		Entry = $$;
	}
		| query 		
	{ 
		makenode(&$$,query);
                link_to_left_child($$,$1);
                link_to_right_child($$,$1);
                link_to_parent($1,$$);
		Entry = $$;
	}
		;

/* not fully implemented yet */

/* utilities create and drop indices, relations (tables), and databases */
utility:	  CREATE UNIQUE INDEX ident ON ident 
		  L_PAREN idx_attrib_list R_PAREN 
	{
		NODPTR leftmost;
		makenode(&$$,create_index);
		makenode(&$2,uniq);
		link_to_left_child($$,$2);
		link_to_right_child($$,$7);
		link_to_parent($2,$$);
		link_to_parent($4,$$);
		link_to_parent($6,$$);
		link_siblings($4,$6);
		link_siblings($2,$4);
		link_all_to_parent($$,$8,&leftmost);
		link_siblings($6,leftmost);
	}	
	  	| CREATE INDEX ident ON ident L_PAREN 
		  idx_attrib_list R_PAREN 
	{
		NODPTR leftmost;
		makenode(&$$,create_index);
		link_to_left_child($$,$3);
		link_to_right_child($$,$7);
		link_to_parent($3,$$);
		link_to_parent($5,$$);
		link_siblings($3,$5);
		link_all_to_parent($$,$7,&leftmost);
		link_siblings($5,leftmost);
	}	
		| DROP INDEX ident
	{
		makenode(&$$,drop_index);
		link_to_left_child($$,$3);
		link_to_right_child($$,$3);
		link_to_parent($3,$$);
	}	
		| CREATE TABLE ident L_PAREN col_data_type_list R_PAREN
			KEY L_PAREN col_list R_PAREN
	{
		NODPTR leftmost;
		NODPTR leftmost_key;
		makenode(&$$,create_table);
		makenode(&$7,key_nod);
		link_to_left_child($$,$3);
		link_to_right_child($$,$5);
		link_to_parent($3,$$);
		link_all_to_parent($$,$5,&leftmost);
		link_siblings($3,leftmost);
		link_to_right_child($$,$9);
		link_all_to_parent($$,$9,&leftmost_key);
		link_siblings($5,$7);
		link_siblings($7,leftmost_key);
	}	
		| LOAD TABLE ident FROM ident
	{
		makenode(&$$,load_table);
		link_to_left_child($$,$3);
		link_to_right_child($$,$5);
		link_to_parent($3,$$);
		link_to_parent($5,$$);
		link_siblings($3,$5);
	}	
		| DROP TABLE ident
	{
		makenode(&$$,drop_table);
		link_to_left_child($$,$3);
		link_to_right_child($$,$3);
		link_to_parent($3,$$);
	}	
		| CREATE DB ident
	{
		makenode(&$$,create_db);
		link_to_left_child($$,$3);
		link_to_right_child($$,$3);
		link_to_parent($3,$$);
	}	
		| DROP DB ident
	{
		makenode(&$$,drop_db);
		link_to_left_child($$,$3);
		link_to_right_child($$,$3);
		link_to_parent($3,$$);
	}	
		| OPEN DB ident
	{
		makenode(&$$,open_db);
		link_to_left_child($$,$3);
		link_to_right_child($$,$3);
		link_to_parent($3,$$);
	}	
		| CLOSE DB ident
	{
		makenode(&$$,close_db);
		link_to_left_child($$,$3);
		link_to_right_child($$,$3);
		link_to_parent($3,$$);
	}	
		| XBEGIN 
	{
		makenode(&$$,xbegin);
	}	
		| XABORT 
	{
		makenode(&$$,xabort);
	}	
		| XCOMMIT 
	{
		makenode(&$$,xcommit);
	}	
		| XPRINT 
	{
		makenode(&$$,xprint);
	}	
		;

/* the update stmt can delete/insert tuples from/into relations and change values of attributes */
update:		  delete_stmt
		| insert_stmt
		| update_stmt
		;

delete_stmt:	  DELETE FROM relation_name where_clause
	{
		makenode(&$$,deletefrom);
		link_to_left_child($$, $3);
		link_to_right_child($$, $4);
		link_to_parent($3, $$);
		link_to_parent($4, $$);
	}
		;
/*
insert_stmt:	  INSERT INTO ident opt_col_list VALUES value_list
*/

insert_stmt:	  INSERT INTO ident VALUES L_PAREN value_list R_PAREN
	{
		NODPTR leftmost;
		makenode(&$$,insert);
		link_to_left_child($$,$3);
		link_to_right_child($$,$6);
		link_to_parent($3,$$);
		link_all_to_parent($$,$6,&leftmost);
	}	
		| INSERT INTO ident query
	{
		makenode(&$$, insert);
		link_to_left_child($$, $3);
		link_to_parent($3, $$);
		link_to_right_child($$, $4);
		link_to_parent($4, $$);
	}
		;

update_stmt:	  UPDATE ident SET update_list where_clause	
	{
		NODPTR leftmost;
		makenode(&$$, update);
		link_to_left_child($$, $2);
		link_to_parent($2, $$);
		link_to_right_child($$, $4);
		link_all_to_parent($$, $4, &leftmost);
		link_to_parent($5, $$);
		link_siblings($5, $2);
	}
		
			/*	ident is relation name    */
		;

update_list:	update_item
	{
		$$ = $1;
	}
		| update_list COMMA update_item
	{
		link_siblings($1, $3);
		$$ = $3;
	}
		;

update_item:	column EQUAL expr
	{
		makenode(&$$, equal);
		link_to_left_child($$, $1);
		link_to_right_child($$, $3);
		link_to_parent($1, $$);
		link_to_parent($3, $$);
	}
		;

/* End of Not Implemented Yet. */

query:		  query_stmt
	{ 
		$$=$1;
	}
		| L_PAREN query R_PAREN set_op L_PAREN query R_PAREN 
			set_order_by_clause	

			/* e.g. (select ...) UNION (select ...) GROUP BY 3  */
	{
                $$ = $4;
                link_to_left_child($$,$2);
                link_to_right_child($$,$8);
                link_to_parent($2,$$);
                link_to_parent($6,$$);
                link_siblings($2,$6);
                link_to_parent($8,$$);
                link_siblings($6,$8);
	}
		;

set_op:		  INTERSECT
	{ 
		makenode(&$$,intersect);
	}
		| MINUS
	{ 
		makenode(&$$,minus);
	}
		| UNION all
	{ 
		makenode(&$$,set_union);
                link_to_left_child($$,$2);
                link_to_right_child($$,$2);
                link_to_parent($2,$$);
	}
		;

/* MGL - order_by_clause flaw...try to fix. */
query_stmt:	  select_clause from_clause where_clause 
			group_by_clause having_clause order_by_clause
        {
		makenode(&$$,query_stmt);
                link_to_left_child($$,$2);
                link_to_right_child($$,$6);
                link_to_parent($1,$$);
                link_siblings($2,$1);
                link_to_parent($2,$$);
                link_siblings($1,$3);
                link_to_parent($3,$$);
                link_siblings($3,$4);
                link_to_parent($4,$$);
                link_siblings($4,$5);
                link_to_parent($5,$$);
		link_siblings($5,$6);
		link_to_parent($6,$$);
        }
		;

select_clause:	  SELECT STAR
        {
                NODPTR dummy_all;
		makenode(&$1,select);
		makenode(&$2,star);
		makenode(&dummy_all,null);
                $$ = $1;
                link_to_right_child($$,$2);
                link_to_left_child($$,dummy_all);
                link_to_parent($2,$$);
                link_to_parent(dummy_all,$$);
                link_siblings(dummy_all,$2);
        }
		| SELECT all select_list
        {
                NODPTR leftmost;
		makenode(&$1,select);
                $$ = $1;
                link_all_to_parent($$,$3,&leftmost);
                link_to_right_child($$,$3);
                link_to_left_child($$,$2);
                link_to_parent($2,$$);
                link_siblings($2,leftmost);
        }
		| SELECT DISTINCT select_list
        {
                NODPTR leftmost;
		makenode(&$1,select);
                $$ = $1;
                link_all_to_parent($$,$3,&leftmost);
                link_to_right_child($$,$3);
		makenode(&$2,distinct);
                link_to_left_child($$,$2);
                link_to_parent($2,$$);
                link_siblings($2,leftmost);
        }
		;

select_list:	  aggregate
	{ 
		$$ = $1;
	}
		| col_name
	{ 
		$$=$1;
	}
		| assigntype
	{
		$$=$1;
	}
		| select_list COMMA col_name
        {
                link_siblings($1,$3);
                $$ = $3;
        }
		| select_list COMMA aggregate
        {
                link_siblings($1,$3);
                $$ = $3;
        }
		| select_list COMMA assigntype
	{
		link_siblings($1,$3);
                $$ = $3;
	}
		;

assigntype:	expr AS col_name
	{
		makenode(&$$, assignment);
		link_to_left_child($$, $1);
		link_to_right_child($$, $3);
		link_to_parent($1, $$);
		link_to_parent($3, $$);
	}
		;

from_clause:	  FROM relation_list
        {
                NODPTR leftmost;
		makenode(&$1,from);
                $$ = $1;
                link_all_to_parent($$,$2,&leftmost);
                link_to_left_child($$,leftmost);
                link_to_right_child($$,$2);
        }
		;

relation_list:	  relation_name
	{ 
		$$ = $1;
	}
		| relation_list COMMA relation_name
        {
                link_siblings($1,$3);
                $$ = $3;
        }
		;

sub_query:
	L_PAREN query R_PAREN 
	{
		$$ = $2;
	}
		;
relation_name:	  ident				/* e.g. deposit */
        {
		$$ = $1;
        }
		| ident dummy_as ident	
						/* e.g. deposit S */
        {
		makenode(&$$,rel_name);
                link_to_left_child($$,$1);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
                link_siblings($1,$3);
        }
		| sub_query dummy_as ident
	{
		makenode(&$$, query_stmt);
                link_to_left_child($$,$1);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
                link_siblings($1,$3);
	}
		| sub_query 
	{
		makenode(&$$, query_stmt);
                link_to_left_child($$,$1);
                link_to_right_child($$,$1);
                link_to_parent($1,$$);
	}

		;

where_clause:	  /* lambda */
	{ 
		makenode(&$$,null);
	}
		| WHERE predicate_list
        {
		makenode(&$1,where);
                $$ = $1;
                link_to_left_child($$,$2);
                link_to_right_child($$,$2);
                link_to_parent($2,$$);
        }
		;


predicate_list:   predicate
	{
		$$ = $1;
	}
		| L_PAREN predicate_list R_PAREN
	{
		$$ = $2;
	}
		| NOT predicate
        {
		makenode(&$1,not);
                $$ = $1;
                link_to_left_child($$,$2);
                link_to_right_child($$,$2);
                link_to_parent($2,$$);
        }
                | NOT L_PAREN predicate_list R_PAREN 
        {
		makenode(&$1,not);
                $$ = $1;
                link_to_left_child($$,$3);
                link_to_right_child($$,$3);
                link_to_parent($3,$$);
        }
		| predicate_list AND predicate
        {
		makenode(&$2,and);
                $$ = $2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
                link_siblings($1,$3);
        }
		| predicate_list AND NOT predicate
        {
		makenode(&$2,and);
		makenode(&$3,not);
                $$ = $2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
                link_siblings($1,$3);
                link_to_left_child($3,$4);
                link_to_right_child($3,$4);
                link_to_parent($4,$3);
        }
                | predicate_list AND L_PAREN predicate_list R_PAREN  
        {
		makenode(&$2,and);
                $$ = $2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$4);
                link_to_parent($1,$$);
                link_to_parent($4,$$);
                link_siblings($1,$4);
        }
		| predicate_list OR  predicate
        {
		makenode(&$2,or);
                $$ = $2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
                link_siblings($1,$3);
        }
		| predicate_list OR  NOT predicate
        {
		makenode(&$2,or);
		makenode(&$3,not);
                $$ = $2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
                link_siblings($1,$3);
                link_to_left_child($3,$4);
                link_to_right_child($3,$4);
                link_to_parent($4,$3);
        }
                | predicate_list OR L_PAREN predicate_list R_PAREN
        {
		makenode(&$2,or);
                $$ = $2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$4);
                link_to_parent($1,$$);
                link_to_parent($4,$$);
                link_siblings($1,$4);
        }
		;

predicate:	  col_ref BETWEEN expr AND expr 
        {
		NODPTR tst1,tst2;
		NODPTR ref2;
		makenode(&$4,and);
		makenode(&tst1,greater_eq);
		makenode(&tst2,less_eq);
		makenode(&ref2,less_eq);
		*(ref2)= *($1);
                $$ = $4;
                link_to_left_child($$,tst1);
                link_to_right_child($$,tst2);
                link_siblings(tst1,tst2);
                link_to_parent(tst1,$$);
                link_to_parent(tst2,$$);
                link_to_left_child(tst1,$1);
                link_to_right_child(tst1,$3);
                link_to_parent($1,tst1);
                link_to_parent($3,tst1);
                link_siblings($1,$3);
                link_to_left_child(tst2,ref2);
                link_to_right_child(tst2,$5);
                link_to_parent(ref2,tst2);
                link_to_parent($5,tst2);
                link_siblings(ref2,$5);
        }
		| col_ref NOT BETWEEN expr AND expr
        {
		NODPTR tst1,tst2;
		NODPTR ref2;
		makenode(&$5,and);
		makenode(&tst1,less);
		makenode(&tst2,greater);
		makenode(&ref2,less_eq);
		*(ref2)= *($1);
                $$ = $5;
                link_to_left_child($$,tst1);
                link_to_right_child($$,tst2);
                link_siblings(tst1,tst2);
                link_to_parent(tst1,$$);
                link_to_parent(tst2,$$);
                link_to_left_child(tst1,$1);
                link_to_right_child(tst1,$4);
                link_to_parent($1,tst1);
                link_to_parent($4,tst1);
                link_siblings($1,$4);
                link_to_left_child(tst2,ref2);
                link_to_right_child(tst2,$6);
                link_to_parent(ref2,tst2);
                link_to_parent($6,tst2);
                link_siblings(ref2,$6);
        }
                | col_ref rel_op expr
        {
                $$ = $2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
                link_siblings($1,$3);
        }                
		| col_name IS NUL
        {
		makenode(&$2,equal);
		makenode(&$3,null);
                $$ = $2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
                link_siblings($1,$3);
	}
		| col_name IS NOT NUL
        {
		makenode(&$2,not_equal);
		makenode(&$3,null);
                $$ = $2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
                link_siblings($1,$3);
        }
		| col_ref rel_op sub_query
	{
                $$ = $2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
		link_siblings($1, $3);
        }

		| col_ref rel_op ALL sub_query
	{
                $$ = $2;
                link_to_left_child($$,$1);
		makenode(&$3, all);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
                link_siblings($1,$3);
		link_to_left_child($3, $4);
		link_to_right_child($3, $4);
		link_to_parent($4, $3);
        }
		| col_ref rel_op ANY sub_query
	{
                $$ = $2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$4);
                link_to_parent($1,$$);
                link_to_parent($4,$$);
        }
		| literal rel_op ANY sub_query
	{
                $$ = $2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$4);
                link_to_parent($1,$$);
                link_to_parent($4,$$);
	}
		| literal rel_op sub_query
	{
		$$ = $2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
	}

		| col_ref NOT IN sub_query
	{
		makenode(&$2, notin);
                $$ = $2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
                link_siblings($1,$3);
	}
		| col_ref NOT MATCH sub_query
	{
		makenode(&$2, notin);
                $$ = $2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
                link_siblings($1,$3);
        }
		| col_ref IN sub_query
	{
		makenode(&$2, in);
                $$ = $2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
                link_siblings($1,$3);
        }
		| col_ref MATCH sub_query
	{
		makenode(&$2, in);
                $$ = $2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
                link_siblings($1,$3);
        }
		| EXISTS sub_query
	{
		makenode(&$1, exists);
                $$ = $1;
                link_to_left_child($$,$2);
                link_to_right_child($$,$2);
                link_to_parent($2,$$);
        }
		;

rel_op:		  EQUAL			{ makenode(&$$,equal); }
		| NOT_EQUAL 		{ makenode(&$$,not_equal); }
		| LESS 			{ makenode(&$$,less); }
		| GREATER		{ makenode(&$$,greater); }
		| LESS_EQUAL		{ makenode(&$$,less_eq); }
		| GREATER_EQUAL		{ makenode(&$$,greater_eq); }
		;

expr: 		  sub_expr_list
	{ 
		$$=$1;
	}
		| sign sub_expr_list	%prec UNARY_ADD
        {
                $$ = $1;
                link_to_left_child($$,$2);
                link_to_right_child($$,$2);
                link_to_parent($2,$$);
        }
		;

sub_expr_list:	  sub_expr
	{ 
		$$ = $1;
	}
		| sub_expr_list arith_op sub_expr
        {
                $$ = $2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
                link_siblings($1,$3);
        }
		;

sub_expr:	  aggregate
	{ 
		$$ = $1;
	}
		| L_PAREN expr R_PAREN
	{ 
		$$ = $2;
	}
		| literal
	{ 
		$$ = $1;
	}
		| col_name      
	{ 
		$$ = $1;
	}
		;

sign:		  SUB		%prec UNARY_ADD		{ makenode(&$$,u_minus); }
		| ADD		%prec UNARY_ADD		{ makenode(&$$,u_plus); }
		;

arith_op:	  ADD		{ makenode(&$$,add_op); }
		| SUB		{ makenode(&$$,sub_op); }
		| STAR		{ makenode(&$$,mul_op); }
		| DIV		{ makenode(&$$,div_op); }
		;

literal:	  INTEGER	{ makenode(&$$,intlit); }
		| FLOAT		{ makenode(&$$,floatlit); }
		| STRING	{ makenode(&$$,strlit); }
		;

aggregate:	  COUNT L_PAREN STAR              R_PAREN
        {
		makenode(&$1,count);
                $$ = $1;
		makenode(&$3,star);
                link_to_left_child($$,$3);
                link_to_right_child($$,$3);
                link_to_parent($3,$$);
        }
		| COUNT L_PAREN DISTINCT col_name R_PAREN
        {
		makenode(&$1,count);
                $$ = $1;
		makenode(&$3,distinct);
                link_to_left_child($$,$3);
                link_to_right_child($$,$4);
                link_to_parent($3,$$);
                link_to_parent($4,$$);
                link_siblings($3,$4);
        }
		| AVG   L_PAREN all expr          R_PAREN
        {
		makenode(&$1,avg);
                $$ = $1;
                link_to_left_child($$,$3);
                link_to_right_child($$,$4);
                link_to_parent($3,$$);
                link_to_parent($4,$$);
                link_siblings($3,$4);
        }
		| AVG   L_PAREN DISTINCT col_name R_PAREN
        {
		makenode(&$1,avg);
                $$ = $1;
		makenode(&$3,distinct);
                link_to_left_child($$,$3);
                link_to_right_child($$,$4);
                link_to_parent($3,$$);
                link_to_parent($4,$$);
                link_siblings($3,$4);
        }
		| MAX   L_PAREN all expr          R_PAREN
        {
		makenode(&$1,max_nod);
                $$ = $1;
                link_to_left_child($$,$3);
                link_to_right_child($$,$4);
                link_to_parent($3,$$);
                link_to_parent($4,$$);
                link_siblings($3,$4);
        }
		| MAX   L_PAREN DISTINCT col_name R_PAREN
        {
		makenode(&$1,max_nod);
                $$ = $1;
		makenode(&$3,distinct);
                link_to_left_child($$,$3);
                link_to_right_child($$,$4);
                link_to_parent($3,$$);
                link_to_parent($4,$$);
                link_siblings($3,$4);
        }
		| MIN   L_PAREN all expr          R_PAREN 
        {
		makenode(&$1,min_nod);
                $$ = $1;
                link_to_left_child($$,$3);
                link_to_right_child($$,$4);
                link_to_parent($3,$$);
                link_to_parent($4,$$);
                link_siblings($3,$4);
        }
		| MIN   L_PAREN DISTINCT col_name R_PAREN
        {
		makenode(&$1,min_nod);
                $$ = $1;
		makenode(&$3,distinct);
                link_to_left_child($$,$3);
                link_to_right_child($$,$4);
                link_to_parent($3,$$);
                link_to_parent($4,$$);
                link_siblings($3,$4);
        }
		| SUM   L_PAREN all expr          R_PAREN
        {
		makenode(&$1,sum);
                $$ = $1;
                link_to_left_child($$,$3);
                link_to_right_child($$,$4);
                link_to_parent($3,$$);
                link_to_parent($4,$$);
                link_siblings($3,$4);
        }
		| SUM   L_PAREN DISTINCT col_name R_PAREN
        {
		makenode(&$1,sum);
                $$ = $1;
		makenode(&$3,distinct);
                link_to_left_child($$,$3);
                link_to_right_child($$,$4);
                link_to_parent($3,$$);
                link_to_parent($4,$$);
                link_siblings($3,$4);
        }
		;

group_by_clause:  /* lambda */
	{ 
		makenode(&$$,null);
	}
		| GROUP BY col_list
        {
                NODPTR leftmost;
		makenode(&$1,group);
                $$ = $1;
                link_all_to_parent($$,$3,&leftmost);
                link_to_right_child($$,$3);
                link_to_left_child($$,leftmost);
        }
		;

having_clause:  /* lambda       */
        { 
                makenode(&$$,null);
        }
                | HAVING predicate_list
        {
                makenode(&$1,having);
                $$ = $1;
                link_to_left_child($$,$2);
                link_to_right_child($$,$2);
                link_to_parent($2,$$);
        }
                ;

order_by_clause:  /* lambda */
	{ 
		makenode(&$$,null);
	}
		| ORDER BY order_by_list
        {
                NODPTR leftmost;
		makenode(&$1,order);
                $$ = $1;
                link_all_to_parent($$,$3,&leftmost);
                link_to_right_child($$,$3);
                link_to_left_child($$,leftmost);
        }
		;

order_by_list:	  col_name   asc_desc
	{
		link_siblings($1,$2);
		$$ = $2;
	}
		| col_number asc_desc
	{
		link_siblings($1,$2);
		$$ = $2;
	}
		| order_by_list COMMA col_name   asc_desc
	{
		link_siblings($1,$3);
		link_siblings($3,$4);
		$$ = $4;
	}
		| order_by_list COMMA col_number asc_desc
	{
		link_siblings($1,$3);
		link_siblings($3,$4);
		$$ = $4;
	}
		;

set_order_by_clause: /*lambda */
	{ 
		makenode(&$$,null);
	}
		| ORDER BY set_order_by_list
        {
                NODPTR leftmost;
		makenode(&$1,order);
                $$ = $1;
                link_all_to_parent($$,$3,&leftmost);
                link_to_right_child($$,$3);
                link_to_left_child($$,leftmost);
        }
		;

set_order_by_list: INTEGER
	{ 
		$$ = $1;
	}
		| set_order_by_list COMMA INTEGER
        {
                link_siblings($1,$3);
                $$ = $3;
        }
		;

asc_desc:	  /* lambda */
	{ 
		makenode(&$$,null);
	}
		| ASC
	{ 
		makenode(&$$,asc);
	}
		| DESC
	{ 
		makenode(&$$,dsc);
	}
		;

/*
opt_col_list:	  */ /* lambda */ /*
	{ 
		makenode(&$$,null);
	}
		| col_list
	{ 
		$$ = $1;
	}
		;
*/

col_list:	col_number
	{ 
		$$ = $1;
	}
		| col_name
	{ 
		$$ = $1;
	}
		| col_list COMMA col_name
        {
                link_siblings($1,$3);
                $$ = $3;
        }
		| col_list COMMA col_number
        {
                link_siblings($1,$3);
                $$ = $3;
        }
		;

col_ref:	  col_name
	{ 
		$$ = $1;
	}
		| aggregate
	{ 
		$$ = $1;
	}
		;

col_name:	  ident DOT ident 		/* e.g.   S.deposit   */
	{ 
		makenode(&$2,dot);
		$$=$2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
                link_siblings($1,$3);
	}
		| ident				/* e.g.     deposit   */
	{ 
		$$=$1;
	}
		;

col_number:	  ident DOT INTEGER		/* e.g.   deposit.2   */
	{ 
		makenode(&$3,intlit);
		makenode(&$2,dot);
		$$=$2;
                link_to_left_child($$,$1);
                link_to_right_child($$,$3);
                link_to_parent($1,$$);
                link_to_parent($3,$$);
                link_siblings($1,$3);
	}
		| INTEGER
	{ 
		makenode(&$1,intlit);
		$$=$1;
	}
		;

column:	          ident
	{ 
		$$=$1;
	}
		| INTEGER
	{ 
		makenode(&$1,intlit);
		$$=$1;
	}
		;

ident:		IDENT
	{ 
		makenode(&$$,id);
	}
		;

/* Utility stuff. Not implemented yet. */

idx_attrib_list:  column asc_desc
	{
			link_siblings($1,$2);
			$$=$2;
	}
		| idx_attrib_list COMMA column asc_desc
	{
			link_siblings($1,$3);
			link_siblings($3,$4);
			$$=$4;
	}
		;

col_data_type_list: ident data_type NOT NUL
	{
			makenode(&$3,null);
			link_siblings($1,$2);
			link_siblings($2,$3);
			$$=$3;
	}
		|   ident data_type
	{
			link_siblings($1,$2);
			$$=$2;
	}
		|   col_data_type_list COMMA ident data_type NOT NUL
	{
			makenode(&$5,null);
			link_siblings($1,$3);
			link_siblings($3,$4);
			link_siblings($4,$5);
			$$=$5;
	}
		|   col_data_type_list COMMA ident data_type
	{
			link_siblings($1,$3);
			link_siblings($3,$4);
			$$=$4;
	}
		;

data_type:	  CHAR_TYPE L_PAREN INTEGER R_PAREN 	
			/* emp_name CHAR(25) */
	{
			makenode(&$$,character_typ);
			makenode(&$3,intlit);
			link_to_left_child($$,$3);
			link_to_right_child($$,$3);
			link_to_parent($3,$$);
	}
		| INT_TYPE			/* age INT */
	{
			makenode(&$$,integer_typ);
	}
		| FLOAT_TYPE			/* salary FLOAT */
	{
			makenode(&$$,float_typ);
	}
		;

value_list:	  value
	{ 
		$$=$1;
	}
		| value_list COMMA value
        {
                link_siblings($1,$3);
                $$ = $3;
        }
		;

value:		  literal
	{ 
		$$=$1;
	}
		| NUL
	{ 
		makenode(&$$,null);
	}
		;

/* end of Utility stuff. */

all:		ALL
	{
		makenode(&$$, all);
	}
		|	/** Lambda	**/	
	{
		makenode(&$$, null);
	}
		;

dummy_as:	AS
	{
		$$ = $1;
	}
		| 	/** Lambda	**/
	{
	}
		;



%%
void yyerror(char */*s*/)
{
   ParserErr.SetState(SysErr::PParseError);
   //exit(0);
}

