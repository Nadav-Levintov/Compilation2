%{
/* Declarations section */
#include <stdio.h>
#include "tokens.h"

typedef enum our_tokens_e
{
	OUR_STARTSTRUCT,
	OUR_ENDSTRUCT,
	OUR_LLIST,
	OUR_RLIST,
	OUR_LDICT,
	OUR_RDICT,
	OUR_KEY,
	OUR_COMPLEXKEY,
	OUR_ITEM,
	OUR_COMMA,
	OUR_TYPE,
	OUR_COMMENT,
	OUR_TRUE,
	OUR_FALSE,
	OUR_INTEGER,
	OUR_INTEGER_HEX,
	OUR_INTEGER_OCT,
	OUR_INTEGER_HEX_ZERO,
	OUR_INTEGER_OCT_ZERO,
	OUR_REAL,
	OUR_STRING1,
	OUR_STRING2,
	OUR_VAL,
	OUR_DECLARATION,
	OUR_DEREFERENCE,
	OUR_EOF1,	
} our_tokens;

enum tokens showToken(our_tokens t);
void errorHandler();
void errorHandlerUnclosed();
void errorHandlerEscaped();

#define MAX_STR_CONST 1025

char string_buf[MAX_STR_CONST];
char *string_buf_ptr;

%}

%option yylineno
%option noyywrap

digit 			([0-9])
letter 			([a-zA-Z])
digitletter 	([a-zA-Z0-9])
whitespace 		([\t\n ])
cr				(\x0d)
lf				(\x0a)
linebreak		(\x0d\x0a|\x0d|\x0a)
hexdigit 		([a-fA-F0-9])
octdigit 		([0-7])
escapeexp 		([\\\"\a\b\e\f\n\r\t\v\0\x{digit}{2}])

%x str1
%x str2

%%

{linebreak}													;
{whitespace}												;
\x2d\x2d\x2d 												return showToken(OUR_STARTSTRUCT);
\.\.\. 														return showToken(OUR_ENDSTRUCT);
\x5b 														return showToken(OUR_LLIST);
\x5d 														return showToken(OUR_RLIST);
\x7b 														return showToken(OUR_LDICT);
\x7d 														return showToken(OUR_RDICT);
\x3a 														return showToken(OUR_KEY);
\x3f 														return showToken(OUR_COMPLEXKEY);
\x2d 														return showToken(OUR_ITEM);
, 															return showToken(OUR_COMMA);
\x21\x21{letter}+ 											return showToken(OUR_TYPE);
\x23[^\x0d\x0a|\x0d|\x0a]+									;
true			 											return showToken(OUR_TRUE);
false			 											return showToken(OUR_FALSE);
[+-]{0,1}{digit}+	 										return showToken(OUR_INTEGER);
0x{hexdigit}+		 										return showToken(OUR_INTEGER_HEX);
0x					 										return showToken(OUR_INTEGER_HEX_ZERO);
0o{octdigit}+		 										return showToken(OUR_INTEGER_OCT);
0o					 										return showToken(OUR_INTEGER_OCT_ZERO);
[+-]{0,1}{digit}+\.{digit}*									return showToken(OUR_REAL);
[+-]{0,1}{digit}*\.{digit}+									return showToken(OUR_REAL);
[+-]{0,1}{digit}*\.{digit}*e[+-]{digit}+					return showToken(OUR_REAL);
\.inf														return showToken(OUR_REAL);
\.NaN														return showToken(OUR_REAL);
{letter}{digitletter}*										return showToken(OUR_VAL);
\'	   														string_buf_ptr = string_buf; BEGIN(str1);
<str1>\'	  												{ /* saw closing quote - all done */
															BEGIN(INITIAL);
															*string_buf_ptr = '\0';
															return showToken(OUR_STRING1);
															}
<str1><<EOF>>												errorHandlerUnclosed();
<str1>[^\']+												{
															char *yptr = yytext;
															while ( *yptr ){
																if((*yptr != '\n') && (*yptr < 32 || *yptr > 126) && (*yptr != '\t'))
																	{
																		strcpy(yytext,yptr);
																		yytext[1]=0;
																		errorHandler();
																	}
																*string_buf_ptr++ = *yptr++;
																}
															}
\"	   														string_buf_ptr = string_buf; BEGIN(str2);
<str2>\"	  												{ /* saw closing quote - all done */
															BEGIN(INITIAL);
															*string_buf_ptr = '\0';
															return showToken(OUR_STRING2);
															}
<str2>\x0d\x0a|\x0d|\x0a									{
															*string_buf_ptr++ = ' ';
															/* Is this correct? */
															}
<str2>\\x[0-9a-fA-F]{2} 									{
															/* hex escape sequence */
															char result;
															(void) sscanf( yytext + 2, "%hhx", &result );
															*string_buf_ptr++ = result;
															}
<str2>\\\\  												*string_buf_ptr++ = '\\';
<str2>\\\"  												*string_buf_ptr++ = '\"';
<str2>\\a  													*string_buf_ptr++ = '\a';
<str2>\\b  													*string_buf_ptr++ = '\b';
<str2>\\e  													*string_buf_ptr++ = '\e';
<str2>\\f  													*string_buf_ptr++ = '\f';
<str2>\\n  													*string_buf_ptr++ = '\n';
<str2>\\r  													*string_buf_ptr++ = '\r';
<str2>\\t  													*string_buf_ptr++ = '\t';
<str2>\\v  													*string_buf_ptr++ = '\v';
<str2>\\0  													*string_buf_ptr++ = '\0';
<str2>\\(.)  												{
																errorHandlerEscaped();
															}
<str2><<EOF>>												errorHandlerUnclosed();
<str2>[^\\|\"|\x0d\x0a|\x0d|\x0a]+							{
															char *yptr = yytext;
															while ( *yptr ){
																if(*yptr < 32 || *yptr > 126)
																{
																	strcpy(yytext,yptr);
																	yytext[1]=0;
																	errorHandler();
																}
																*string_buf_ptr++ = *yptr++;
																}
															}
&{letter}+													return showToken(OUR_DECLARATION);
\*{letter}+													return showToken(OUR_DEREFERENCE);
<<EOF>>														return showToken(OUR_EOF1);
.															errorHandler();


%%

enum tokens showToken(our_tokens t)
{
	switch (t){
	case OUR_STARTSTRUCT:
		return STARTSTRUCT;
		break;
	case OUR_ENDSTRUCT :
		return ENDSTRUCT;
		break;
	case OUR_LLIST     :
		return LLIST;
		break;
	case OUR_RLIST     :
		return RLIST;
		break;
	case OUR_LDICT     :
		return LDICT;
		break;
	case OUR_RDICT     :
		return RDICT;
		break;
	case OUR_KEY       :
		return KEY;
		break;
	case OUR_COMPLEXKEY:
		return COMPLEXKEY;
		break;
	case OUR_ITEM      :
		return ITEM;
		break;
	case OUR_COMMA     :
		return COMMA;
		break;
	case OUR_TYPE      :
		return TYPE;
		break;
	case OUR_COMMENT   :
		/* Do not do anything */
		break;
	case OUR_TRUE      :
		return TRUE;	
		break;
	case OUR_FALSE     :
		return FALSE;
		break;
	case OUR_INTEGER_HEX	:
	case OUR_INTEGER_OCT	:
	case OUR_INTEGER_HEX_ZERO	:
	case OUR_INTEGER_OCT_ZERO	:
	case OUR_INTEGER   :
		return INTEGER;
		break;
	case OUR_REAL     :
		return REAL;
		break;
	case OUR_STRING1   :
	case OUR_STRING2   :
		return STRING;
		break;
	case OUR_VAL       :
		return VAL;
		break;
	case OUR_DECLARATION:
		return DECLARATION;
		break;
	case OUR_DEREFERENCE:
			return DEREFERENCE;
		break;
	case OUR_EOF1	    :
		return EF;
		break;
	default:
		errorHandler();
	}
	
}

void errorHandler()
{
	printf("Error %s\n", yytext);
	exit(0);
}

void errorHandlerUnclosed()
{
	printf("Error unclosed string\n");
	exit(0);
}

void errorHandlerEscaped()
{
	printf("Error undefined escape sequence %c\n",yytext[1]);
	exit(0);
}
