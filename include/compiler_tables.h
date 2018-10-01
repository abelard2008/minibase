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
*      token.h : external definitions for SQL tokens                       *
*                                                                              *
*******************************************************************************/

#ifndef COMPILER_TABLES_H
#define COMPILER_TABLES_H

#include "hashing.h"


/* Limit on no. of tokens. */

#define pp_MAX_TOKENS	10000
#define pp_TOKEN_QTY 28 

/* The entry for a token seen. */
typedef struct {
	int start_col;
	int start_line;
	int token_type;
} pp_TOKEN_ENTRY;

/* The token array. */

extern pp_TOKEN_ENTRY *pp_token_table[];

/* The current token. */

typedef struct {
	pp_TOKEN_ENTRY tok_stuff;
	int maxflag;
	union {
		int int_val;
		double *float_val;		
		char *str_val;
		pp_ID_ENTRY * id_val;
	} actual_value;
} pp_TOKEN_INFO;

extern pp_TOKEN_INFO *pp_current_tok;
extern int pp_current_tok_no;


extern pp_ID_ENTRY *pp_find_id_ptr(int);
extern int pp_id_string_insert(char *,pp_ID_ENTRY *);
extern int pp_table_init (void);

#endif /* COMPILER_TABLES_H */
