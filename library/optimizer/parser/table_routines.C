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
*      table_routines.c : compiler tables for SQL                              *
*                                                                              *
*******************************************************************************/

/* Routines for accessing the id_table, symbol tables 
   and other compiler tables. */

/* Include headers. */

#include <stdio.h>
#include <ext_globals.h>
#include <sym_tabs.h>
#include <ext_token.h>

extern FILE f1;

/* Initialize compiler tables. */

void table_init()
{

/* Locals. */
/* no longer used  - mgl
	int i,j; */

/* Initialize current_tok. */

	current_tok = ( TOKEN_INFO *) malloc (sizeof (TOKEN_INFO));

	current_tok->tok_stuff.start_col = 1;
	current_tok->tok_stuff.start_line = 1;
	current_tok->tok_stuff.token_type = EOF;

}

/* Inserting an identifier string into a given index
   position in the id_table, given the index in the
   bucket overflow format. */

int id_string_insert(char *insert_str,ID_ENTRY *ptr)
{
	current_id_no++;
	ptr->id_no = current_id_no;
	strcpy(ptr->id_name,insert_str);
	id_no_match[current_id_no-1] = ptr;
	return(current_id_no);

}

/* Find ptr for a given hash value. */

ID_ENTRY *find_id_ptr(int val)
{

/* Locals. */

	int i;
	ID_ENTRY *temp_ptr;

/* Traverse to input point. */

	if ( val % 10 == 0 )
	{
		temp_ptr = id_table[val/10];
	}
	else
	{
		temp_ptr = id_table[val/10];

		for(i=0;i<(val % 10);i++)
		{
			temp_ptr=temp_ptr->next;
		}
	}

/* Return pointer. */

	return (temp_ptr);

}
