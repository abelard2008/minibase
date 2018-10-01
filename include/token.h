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

	Token definitions for SQL Translator

	CS764 Course Project

		Ted Hill
		Manuvir Das			 */

/* Limit on no. of tokens. */

#ifndef _TOKEN_H
#define _TOKEN_H

#define MAX_TOKENS	10000
#define TOKEN_QTY 28 

/* The entry for a token seen. */
typedef struct {
	int start_col;
	int start_line;
	int token_type;
} TOKEN_ENTRY;

/* The token array. */

TOKEN_ENTRY *token_table[MAX_TOKENS];

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

TOKEN_INFO *current_tok;

int current_tok_no = 0;


#endif
