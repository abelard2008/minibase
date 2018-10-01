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
*      ext_sym_tabs.h : external definitions for the compiler symbol tables    *
*                                                                              *
*******************************************************************************/

#ifndef EXT_SYM_TABS_H 
#define EXT_SYM_TABS_H

/* Definitions for the symbol tables. */

/* The maximum number of ids in a source program. */

#define MAX_IDS	100

/* An entry in the id table. */

typedef struct id_entry {
	char id_name[300];
	int id_no;
	struct id_entry *next;
} ID_ENTRY;

/* The id table ( bucket overflow ) */

extern ID_ENTRY *id_table[];

/* A table for pointers using the id no. of an identifier. */

extern ID_ENTRY *id_no_match[];

/* A function defn. */

extern ID_ENTRY *find_id_ptr();

#endif
