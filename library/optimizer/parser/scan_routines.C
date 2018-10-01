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
*      scan_routines.c : utility routines for SQL lexical analyzer             *
*                                                                              *
*******************************************************************************/

/* Include system headers. */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

/* Include header files of compiler. */

#include <ext_globals.h>
#include <ext_sym_tabs.h>
#include <ext_token.h>
#include "y.tab.h"
#include <arithops.h>
#include <mr_syserr.h>

extern SysErr ParserErr;

extern int strcmp_ours(char *,char *);

char *remove_underscores(char *) ;

/* Include function definitions. */

#include <scan_func_defs.h>

/* Routines invoked by the flex listing of the scanner. */

/* For all tokens other than id,intlit,strlit,reallit */

int note_token(int token)
{

/* Locals. */
/* i not needed
	int i;

*/

/* Make an entry for the token in the token table. */

	current_tok_no++;

	token_table[current_tok_no-1] = (TOKEN_ENTRY *) malloc (sizeof (TOKEN_ENTRY));

	token_table[current_tok_no-1]->start_col = col_no;
	token_table[current_tok_no-1]->start_line = line_no;
	token_table[current_tok_no-1]->token_type = token;

        col_no += yyleng;

	current_tok->tok_stuff = *(token_table[current_tok_no-1]);
	
	return(current_tok_no-1);

}

/* Handling the newline. */

int newline()
{

/* Adjust line no. and col. no. */

        line_no++;
        col_no=1;
	return(line_no);
}

/* Action for integer literal. */

int int_token()
{

/* Locals. */

	int value=0 ;

/* Note the token. */

	note_token(INTEGER);

/* Find integer value and store in current token. */

	value = get_int_val(yytext) ;

	if( value==-1) {
	  ParserErr.SetState(SysErr::PIntOverflow);
		//printf("error line %d column %d : integer overflow\n",line_no, col_no-1);
		//	no_error = 0; 
	}
	if( value==-2) {  /* This is MAXINT+1. Set a flag in cur_tok ManuD */ 
	}

/* Store intval ****/ 

	current_tok->actual_value.int_val= value;
	return(value);

}

/* Find out integer value. */

int get_int_val( char *text_under ) 
{

/* Locals. */

	int number=0 ;
	char *text ; 
	 
/* Remove underscores. */

	text = (char *)malloc((sizeof (char))*300);	
	strcpy(text,remove_underscores(text_under)) ;

/* Find int value of number. */

	number = our_atoi(text) ; 
	free(text); 
	return(number);

}

/* Remove underscores. */

char *remove_underscores(char * str ) 
{

/* Locals. */

	int i=0,j=0 ;
	char *num_str; 

	num_str = (char *)malloc((sizeof (char))*300);	
/* Remove underscores. */

	while( *(str+i) != '\0' ) 
	{

		while( *(str+i) == '_' ) 
		{
			i++ ;
		}

		*(num_str+j) = *(str+i) ;
		j++ ;
		i++ ; 

	} 

	*(num_str+j) = '\0' ;
	return(num_str) ;

}

extern "C" int strcasecmp(const char* s1, const char* s2);

int strcmp_ours(char *s,char *t)
{
        if ((s==NULL || t==NULL) && s != t)
                return 1;
        else
                if ((s==NULL || t==NULL) && s == t)
                        return 0;
                else
                        return strcasecmp(s,t);
}

/* Conversion of string to integer. */

int our_atoi(char *s )
{

/* Locals. */

	int i=0 ; long int n=0 ;
	long int diff=0; long int prod=0 ;
	status err_flag=OK; 

/* Conversion. */

	if(!strcmp_ours(s,"2147483648"))
		return(-2);

	for(i=0 ; s[i]>='0' && s[i]<='9' ; ++i)
	{
		integerop(MulOper,10,n,&prod,&err_flag);
		if( yyleng >= 10 && err_flag == Overflow)
			return(-1); 
		integerop(SubOper,s[i],'0',&diff,&err_flag); 
		integerop(AddOper,prod,diff,&n,&err_flag); 
		if( yyleng >= 10 && err_flag == Overflow)
			return(-1); 
		/* n = 10*n+(s[i]-'0') ; */ 
	}

	return(n) ; 

} 

/* Action for real literal. */

int real_token()
{

/* Locals. */

	double  value ;
	char *text; 

	text = (char *)malloc((sizeof (char))*300);	

/* Note the token. */

	note_token(FLOAT);

/* Find real value and store in current token. */

	  strcpy(text,remove_underscores(yytext)); 
	  value = atof(text) ; 
	  if(value==0){
		if(!is_zero(text)) {
		  ParserErr.SetState(SysErr::PFloatUnderflow);
			//printf("error line %d column %d : floating point underflow \n",line_no, col_no);
			//no_error = 0; 
		}
	  }
	  else if(value==HUGE) {  /* Check out the special case */ 
	    ParserErr.SetState(SysErr::PFloatOverflow);
		//printf("error line %d column %d : floating point overflow \n",line_no, col_no);
			//no_error = 0;
	  }

	free(text); 

/* Store real val ****/ 

	current_tok->actual_value.float_val=(double *)malloc(sizeof (double));
	*(current_tok->actual_value.float_val)=value;
	return((int) value);

}

/* Action for string literal. */

int str_token()
{

	char temp_str[256];
	int i=1;

/* Note the token. */

	note_token(STRING);

/* Find string value and store in current token. */

	current_tok->actual_value.str_val = (char *)malloc(sizeof(char)*256);

	while ( yytext[i] != '"' )
	{
		temp_str[i-1] = yytext[i];
		i++;
	}
	temp_str[i-1] = '\0';

	strcpy( current_tok->actual_value.str_val,temp_str);
	return(i);

}

int is_zero(char *str)
{
	int i=0, flag=1 ;

	while(*(str+i)!='e' && *(str+i)!='E' && *(str+i)!='\0' ) {
		if(*(str+i)!='0' && *(str+i)!='.'){
			flag=0;
			break;
		}
		i++;
	}
	return(flag);
}

/* Action for identifier. */

int id_token()
{

/* Locals. */

	int hash_val=0 ;
	ID_ENTRY *id_ptr;

if( yyleng > MAXTOKLEN )
{
	return(IDENT);
}
else
{

/* Find hash value. */

	hash_val = hash(makeupper(yytext));

/* Find pointer to ID table. */

	id_ptr = find_id_ptr(hash_val);

/* Note the token. */

	note_token(IDENT);

	current_tok->actual_value.id_val = id_ptr;

	if ( strcmp_ours(makeupper(yytext),makeupper(id_ptr->id_name)) != 0 )
	{
		id_string_insert(yytext,id_ptr) ;
	}

	return(IDENT);
}

}


/* Return upper case value of string. */

char *makeupper(char *stri)
{

/* Locals. */

	char *upper_str ;
	int i=0 ;

	upper_str = (char *) malloc ((sizeof (char))*500);	
	*(upper_str) = '\0' ;
/* Convert. */

	while( *(stri+i) != '\0' )
	{
	     if( !isupper(*(stri+i)) && isalpha(*(stri+i))) 
		*(upper_str+i) = toupper(*(stri+i)) ;
	     else 
		*(upper_str+i) = *(stri+i) ;
	     i++ ;
	}

	*(upper_str+i) = '\0' ;

	return(upper_str) ;

}
