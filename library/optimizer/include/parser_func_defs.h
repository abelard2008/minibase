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

//extern "C" void yyerror(char *);
//extern "C" int yylex();
extern void yyerror(char *);
extern int yylex();

extern makenode(NODPTR *,NODE_TYPE);
extern void link_to_left_child(NODPTR,NODPTR);
extern void link_to_right_child(NODPTR,NODPTR);
extern void link_to_parent(NODPTR,NODPTR);
extern void link_siblings(NODPTR,NODPTR);
extern void link_all_to_parent(NODPTR,NODPTR,NODPTR *);

