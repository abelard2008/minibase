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
*      ext_token.h : external definitions for SQL tokens                       *
*                                                                              *
*******************************************************************************/

#ifndef _EXT_TOKEN_H
#define _EXT_TOKEN_H

/* Limit on no. of tokens. */

#define MAX_TOKENS	10000
#define TOKEN_QTY 28 

/* The entry for a token seen. */
typedef struct {
	int start_col;
	int start_line;
	int token_type;
} TOKEN_ENTRY;

/* The token array. */

extern TOKEN_ENTRY *token_table[];

/* The current token. */

typedef struct {
	TOKEN_ENTRY tok_stuff;
	int maxflag;
	union {
		int int_val;
		double *float_val;		
		char *str_val;
		ID_ENTRY * id_val;
	} actual_value;
} TOKEN_INFO;

extern TOKEN_INFO *current_tok;

extern int current_tok_no;

#endif
