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

#ifndef _SCAN_FUNC_DEFS_H
#define _SCAN_FUNC_DEFS_H

extern newline();
extern note_token(int);
extern int_token();
extern real_token();
extern str_token();
extern int id_token();
extern int get_int_val(char *);
extern int our_atoi(char *);
extern int is_zero(char *);
extern int hash(char *);
extern ID_ENTRY *find_id_ptr(int);
extern id_string_insert(char *,ID_ENTRY *);
extern char *makeupper(char *); 
extern void integerop(oper , long int , long int , long int *, status *);

#endif
