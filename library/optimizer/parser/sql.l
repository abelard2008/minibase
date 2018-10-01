%{

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
*      sql.l : lexical analyzer for SQL                                        *
*                                                                              *
*******************************************************************************/

/* This file contains the lex specification that rcognizes the tokens
   or lexicon of SQL. This is the part of the processor that breaks up
   a stream of SQL text into a stream of tokens that combine syntactically
   to form an instance of grammatically correct SQL. */

/* Unfortunately it is not possible to insert comments arbitrarily in a lex
   spec., but the file is self-explanatory. A pattern is recognized, and
   the appropriate token is created and filled with information, and then
   sent on to the parser. */

/* Include information about token structure and global variables. */

#include <stdio.h>
#include <string.h>
#include "y.tab.h"
#include <ext_globals.h>
#include <ext_sym_tabs.h>
#include <token.h>
#include <scanner_func_defs.h>
#include <mr_syserr.h>

extern SysErr ParserErr;

extern char *input_string;
#define YY_NO_UNPUT
#undef YY_INPUT
#define YY_INPUT(buf,result,maxsize) \
                   result = strlen (input_string); \
                   if (result > maxsize) result = maxsize; \
                   strncpy (buf, input_string, maxsize); \
                   input_string += result;

%}

us              _
digit           [0-9]
alpha           [a-zA-Z]
character       {digit}|{us}|{alpha}

int_lit         {digit}({digit}|{us})*
real_lit        {int_lit}(\.{int_lit})?([Ee][\+\-]?{int_lit})?
str_lit         \"([^\"\n]|(\"\"))*\"

identifier      {alpha}{character}*

abort           [aA][bB][oO][rR][tT]
all		[aA][lL][lL]
and		[aA][nN][dD]
any		[aA][nN][yY]
as		[aA][sS]
asc		[aA][sS][cC]
avg		[aA][vV][gG]
between		[bB][eE][tT][wW][eE][eE][nN]
begin           [bB][eE][gG][iI][nN]
by		[bB][yY]
char		[cC][hH][aA][rR]
close		[cC][lL][oO][sS][eE]
commit          [cC][oO][mM][mM][iI][tT]
count		[cC][oO][uU][nN][tT]
create		[cC][rR][eE][aA][tT][eE]
db		[dD][bB]
delete		[dD][eE][lL][eE][tT][eE]
desc		[dD][eE][sS][cC]
distinct	[dD][iI][sS][tT][iI][nN][cC][tT]
drop		[dD][rR][oO][pP]
exists		[eE][xX][iI][sS][tT][sS]
float		[fF][lL][oO][aA][tT]
from		[fF][rR][oO][mM]
group		[gG][rR][oO][uU][pP]
having		[hH][aA][vV][iI][nN][gG]
in		[iI][nN]
index		[iI][nN][dD][eE][xX]
insert		[iI][nN][sS][eE][rR][tT]
integer		[iI][nN][tT][eE][gG][eE][rR]
intersect	[iI][nN][tT][eE][rR][sS][eE][cC][tT]
into		[iI][nN][tT][oO]
is		[iI][sS]
key		[kK][eE][yY]
load		[lL][oO][aA][dD]
match		[mM][aA][tT][cC][hH]
max		[mM][aA][xX]
min		[mM][iI][nN]
minus		[mM][iI][nN][uU][sS]
not		[nN][oO][tT]
null		[nN][uU][lL][lL]
on		[oO][nN]
open		[oO][pP][eE][nN]
or		[oO][rR]
order		[oO][rR][dD][eE][rR]
select		[sS][eE][lL][eE][cC][tT]
set		[sS][eE][tT]
sum		[sS][uU][mM]
table		[tT][aA][bB][lL][eE]
union		[uU][nN][iI][oO][nN]
unique		[uU][nN][iI][qQ][uU][eE]
update		[uU][pP][dD][aA][tT][eE]
values		[vV][aA][lL][uU][eE][sS]
where		[wW][hH][eE][rR][eE]
tprint		[tT][pP][rR][iI][nN][tT]

%%

[ ]             {  }
\t              {  }
\n              { newline(); }

{integer}	{ note_token(INT_TYPE);		return(INT_TYPE); }
{float}		{ note_token(FLOAT_TYPE);	return(FLOAT_TYPE); }
{char}		{ note_token(CHAR_TYPE);	return(CHAR_TYPE); }

{create} 	{ note_token(CREATE); 	return(CREATE); }
{load} 		{ note_token(LOAD); 	return(LOAD); }
{drop}		{ note_token(DROP);	return(DROP); }
{open}		{ note_token(OPEN);	return(OPEN); }
{close}		{ note_token(CLOSE);	return(CLOSE); }
{db}		{ note_token(DB);	return(DB); }
{table}		{ note_token(TABLE);	return(TABLE); }
{unique} 	{ note_token(UNIQUE); 	return(UNIQUE); }
{index} 	{ note_token(INDEX);	return(INDEX); }
{on}		{ note_token(ON); 	return(ON); }
{delete}	{ note_token(DELETE);	return(DELETE); }
{insert}	{ note_token(INSERT);	return(INSERT); }
{into}		{ note_token(INTO);	return(INTO); }
{values}	{ note_token(VALUES);	return(VALUES); }
{update}	{ note_token(UPDATE);	return(UPDATE); }
{set}		{ note_token(SET);	return(SET); }
{key}		{ note_token(KEY);	return(KEY); }

{select}	{ note_token(SELECT);	return(SELECT); }
{from}		{ note_token(FROM);	return(FROM); }
{where}		{ note_token(WHERE);	return(WHERE); }
{group}		{ note_token(GROUP);	return(GROUP); }
{order}		{ note_token(ORDER);	return(ORDER); }
{having}	{ note_token(HAVING);	return(HAVING); }
{by}		{ note_token(BY);	return(BY); }
{as}		{ note_token(AS);	return(AS); }

{begin}		{ note_token(XBEGIN);	return(XBEGIN); }
{commit}	{ note_token(XCOMMIT);	return(XCOMMIT); }
{abort}	        { note_token(XABORT);	return(XABORT); }
{tprint}	{ note_token(XPRINT);	return(XPRINT); }

{asc}		{ note_token(ASC);	return(ASC); }
{desc}		{ note_token(DESC);	return(DESC); }

{intersect}	{ note_token(INTERSECT);return(INTERSECT); }
{minus}		{ note_token(MINUS);	return(MINUS); }
{union}		{ note_token(UNION);	return(UNION); }

{count}		{ note_token(COUNT);	return(COUNT); }
{avg}		{ note_token(AVG);	return(AVG); }
{max}		{ note_token(MAX);	return(MAX); }
{min}		{ note_token(MIN);	return(MIN); }
{sum} 		{ note_token(SUM);	return(SUM); }
{all}		{ note_token(ALL);	return(ALL); }
{distinct}	{ note_token(DISTINCT);	return(DISTINCT); }
{any}		{ note_token(ANY);	return(ANY); }
{in}		{ note_token(IN);	return(IN); }
{match}		{ note_token(MATCH);	return(MATCH); }
{exists}	{ note_token(EXISTS);	return(EXISTS); }

{not}		{ note_token(NOT);	return(NOT); }
{null} 		{ note_token(NUL); 	return(NUL); }
{and}		{ note_token(AND);	return(AND); }
{or}		{ note_token(OR);	return(OR); }
{is}		{ note_token(IS);	return(IS); }
{between} 	{ note_token(BETWEEN); 	return(BETWEEN); }

"="		{ note_token(EQUAL); 		return(EQUAL); }
"<>"		{ note_token(NOT_EQUAL); 	return(NOT_EQUAL); }
"<"		{ note_token(LESS); 		return(LESS); }
">"		{ note_token(GREATER);		return(GREATER); }
"<="		{ note_token(LESS_EQUAL);	return(LESS_EQUAL); }
">="		{ note_token(GREATER_EQUAL);	return(GREATER_EQUAL); }

"("		{ note_token(L_PAREN);	return(L_PAREN); }
")"		{ note_token(R_PAREN);	return(R_PAREN); }

","		{ note_token(COMMA);	return(COMMA); }
"."		{ note_token(DOT);	return(DOT); }

"+"		{ note_token(ADD);	return(ADD); }
"-"		{ note_token(SUB);	return(SUB); }
"*"		{ note_token(STAR);	return(STAR); }
"/"		{ note_token(DIV);	return(DIV); }


{int_lit}       { int_token(); return(INTEGER); }
{real_lit}      { real_token(); return(FLOAT); }

{str_lit}       { str_token(); return(STRING); }

{identifier}    { id_token(); return(IDENT); }


.               { ParserErr.SetState(SysErr::PIllegalChars); }

%%

void yy_restart_lexical_analyzer (void) {
// and the end of a query, the scanner gets an YY_NULL 
// and unfortunately decides to remember it so that
// yylex returns 0 on all subsequent calls.
// we need some way of preventing this behaviour.
// i.e. we need to return a YY_NULL at the end of
// each query but, then at the start of the next
// query we also need the scanner to start all over
// again. so this is a hack to achieve that.

// if someone has a better way of doing this, 
// go ahead, be my guest!
yy_init = 1;
}







