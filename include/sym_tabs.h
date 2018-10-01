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

	Compiler tables for SQL Translator

	CS764 Course Project

		Ted Hill
		Manuvir Das			 */

/* Definitions for the symbol tables. */

#ifndef _SYM_TABS_H
#define _SYM_TABS_H

/* The maximum number of ids in a source program. */

#define MAX_IDS	100

/* An entry in the id table. */

typedef struct id_entry{
	char id_name[300];
	int id_no;
	struct id_entry *next;
} ID_ENTRY;

/* The id table ( bucket overflow ) */

ID_ENTRY *id_table[63];

/* A table for pointers using the id no. of an identifier. */

ID_ENTRY *id_no_match[MAX_IDS];

/* A function defn. */

extern ID_ENTRY *find_id_ptr();

#endif
