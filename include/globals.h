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
*      globals.h : gloabls definitions for the system                          *
*                                                                              *
*******************************************************************************/

#ifndef _GLOBALS_H
#define _GLOBALS_H

/* Flag for stopping processing at any time. */
#include <stdio.h>

int abort_flag = 0;

/* Flag for main loop at top-level. */

int continue_processing = 1;

/* Some constants used by the compiler. */

#define	NO	0
#define	YES	1

#define MAXTOKLEN 300 

/* Keeping track of input location. */

int line_no = 1;
int col_no = 1;
//extern FILE *yyin ; 
//extern char yytext[] ; 
//extern int yy_init;

//extern yyparse() ;

/* Keeping track of serial numbers for the identifiers. */

int current_id_no = 0;

/* Check for error status. */

// int no_error = 1;

/* Debug and execute flags. */

int execute_op = 1;
int execute_query = 1;
int print_op_tree = 0;
int print_query_tree = 0;
int batch_mode = 0;

/* Batch file string. */

char user_batch_file[100];

/* Optimization time. */

long opt_time=0;

#endif
