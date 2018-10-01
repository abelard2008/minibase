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
*      hash_routines.c : hash table for identifiers (field/relation names)     *
*                                                                              *
*******************************************************************************/

/* The hashing formula ( bucket-overflow method ) is
   as follows:-

   Buckets are numbered 0,10,20,30,40, etc
   Overflow in the buckets is handled thru buck+1,buck+2 etc.

   Hash function mapping is computed by taking ascii values of 
   all characters and summing them up, mod 63. Thus buckets go
   from 0 to 620. 

   */

/* Include files/headers. */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <ext_globals.h>
#include <ext_sym_tabs.h>

#include <malloc.h>
#include <string.h>

#include <assert.h>

extern int strcmp_ours(char *,char *);

/* Conversion function. */

extern char *makeupper(char *); 

/* The main hash function. */

int hash(char *hash_str)
{

/* Local variables. */

	int i=0;
	int hash_value=0;
	ID_ENTRY *temp_ptr;

/* Read in string character by character and convert to upper case,
   then apply hashing formula. */

	while ( *(hash_str+i) != '\0' )
	{
		hash_value+=(*(hash_str+i));
		i++;
	}

/* Compute table index. */

	hash_value = ( hash_value % 63 ) * 10 ;
	temp_ptr = id_table[hash_value/10];

	if ( temp_ptr == NULL )
	{
		id_table[hash_value/10]=(ID_ENTRY *) 
				malloc(sizeof (ID_ENTRY));
		
		assert(id_table[hash_value/10]);
		// clear it out, maybe that will help

		id_table[hash_value/10]->id_name[0] = 0;
		id_table[hash_value/10]->id_no = 0;
		id_table[hash_value/10]->next = 0;		
	}
	else
	{
		if ( strcmp_ours(makeupper(temp_ptr->id_name),hash_str) != 0 )
		{

			while ( temp_ptr && temp_ptr->next != NULL && 
				strcmp_ours(makeupper(temp_ptr->id_name),hash_str) != 0)
			{
				hash_value++;
				temp_ptr = temp_ptr->next;
			}

			if (temp_ptr && strcmp_ours(makeupper(temp_ptr->id_name),hash_str) != 0)
			{
				temp_ptr->next=(ID_ENTRY *) 
					malloc (sizeof (ID_ENTRY));
				temp_ptr->next->id_name[0] = 0;
				temp_ptr->next->id_no = 0;
				temp_ptr->next->next = NULL;
				hash_value++; 
			}
		}
	}

/* return hash_value. */


	return ( hash_value );

}
