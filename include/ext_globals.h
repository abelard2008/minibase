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
*      ext_globals.h : external global definitions for the system              *
*                                                                              *
*******************************************************************************/

#ifndef _EXT_GLOBALS_H
#define _EXT_GLOBALS_H

#include <stdio.h> 
#include <stdlib.h>
#include <malloc.h>

/* Flag for stopping processing at any time. */

extern int abort_flag;

/* Flag for main loop at top-level. */

extern int continue_processing;

/* Some constants used by the compiler. */

#define	NO	0
#define	YES	1
#define BLANK   255 
#define TAB     256 

#define MAXTOKLEN 300

#define RESERVEDNO  42 

/* Keeping track of input location. */

extern int line_no;
extern int col_no;

extern char *yytext; 
extern int yyleng ; 
extern FILE *fp ; 

/* Keeping track of serial numbers for the identifiers. */

extern int current_id_no;

/* Check for error status. */

// extern int no_error;

/* Debug and execute flags. */

extern int execute_op;
extern int execute_query;
extern int print_op_tree;
extern int print_query_tree;
extern int batch_mode;

/* Batch file string. */

extern char user_batch_file[];

/* Optimization time. */

extern long opt_time;

#endif
