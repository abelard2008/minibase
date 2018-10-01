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
*      sym_tabs.h : external definitions for the compiler symbol tables    *
*                                                                              *
*******************************************************************************/

#ifndef HASHING_H // added by Navin Kabra: 6/16/93
#define HASHING_H // for obvious reasons

/* Definitions for the symbol tables. */

/* The maximum number of ids in a source program. */

#define pp_MAX_IDS	100

/* An entry in the id table. */

typedef struct pp_id_entry {
	char id_name[300];
	int id_no;
	struct pp_id_entry *next;
} pp_ID_ENTRY;

/* The id table ( bucket overflow ) */

extern pp_ID_ENTRY *pp_id_table[];

/* A table for pointers using the id no. of an identifier. */

extern pp_ID_ENTRY *pp_id_no_match[];

/* A function defn. */

extern pp_ID_ENTRY *pp_find_id_ptr();


extern int pp_hash(char *);

#endif
