%{
// $Id: scanner.l,v 1.4 2014-11-15 18:37:02-08 - - $

#include "auxlib.h"
#include "lyutils.h"
#include "yyparse.h"

#define YY_USER_ACTION  { scanner_useraction (); }
#define IGNORE(THING)   { }

%}

%option 8bit
%option debug
%option nodefault
%option nounput
%option noyywrap
%option verbose
%option warn

LETTER          [A-Za-z_]
DIGIT           [0-9]
INTEGER          (({DIGIT})+)
CHARACTER       ('([^\\'\n]|\\[\\'"0nt])')
STRING          (\"([^\\"\n]|\\[\\'"0nt])*\")
IDENT           ({LETTER}({LETTER}|{DIGIT})*)
NOTIDENT        ({DIGIT}({LETTER}|{DIGIT})*)
NOTCHAR         ('([^\\'\n]|\\[\\'"0nt]))
NOTSTRING       (\"([^\\"\n]|\\[\\'"0nt])*)

%%

"#".*           { scanner_include(); }
[ \t]+          { IGNORE (white space) }
\n              { scanner_newline(); }

"void"            { return yylval_token (TOK_VOID); }
"bool"            { return yylval_token (TOK_BOOL); }
"char"            { return yylval_token (TOK_CHAR); }
"int"             { return yylval_token (TOK_INT); }
"string"          { return yylval_token (TOK_STRING); }
"struct"          { return yylval_token (TOK_STRUCT); }
"if"              { return yylval_token (TOK_IF); }
"else"            { return yylval_token (TOK_ELSE); }
"while"           { return yylval_token (TOK_WHILE); }
"return"          { return yylval_token (TOK_RETURN); }
"false"           { return yylval_token (TOK_FALSE); }
"true"            { return yylval_token (TOK_TRUE); }
"null"            { return yylval_token (TOK_NULL); }
"ord"             { return yylval_token (TOK_ORD); }
"chr"             { return yylval_token (TOK_CHR); }
"new"             { return yylval_token (TOK_NEW); }



"[]"            { return yylval_token (TOK_NEWARRAY); }
"["             { return yylval_token ('['); }
"]"             { return yylval_token (']');}
"{"             { return yylval_token ('{'); }
"}"             { return yylval_token ('}'); }
","             { return yylval_token (','); }
"."             { return yylval_token ('.'); }
"=="            { return yylval_token (TOK_EQ); }
"!="            { return yylval_token (TOK_NE); }
"<"             { return yylval_token (TOK_LT); }
"<="            { return yylval_token (TOK_LE); }
">"             { return yylval_token (TOK_GT); }
">="            { return yylval_token (TOK_GE); }
"%"             { return yylval_token ('%'); }
"!"             { return yylval_token ('!'); }


"="             { return yylval_token ('='); }
"+"             { return yylval_token ('+'); }
"-"             { return yylval_token ('-'); }
"*"             { return yylval_token ('*'); }
"/"             { return yylval_token ('/'); }
"^"             { return yylval_token ('^'); }
"("             { return yylval_token ('('); }
")"             { return yylval_token (')'); }
";"             { return yylval_token (';'); }


{CHARACTER}     { return yylval_token (TOK_CHARCON); }
{STRING}        { return yylval_token (TOK_STRINGCON); }
{INTEGER}       { return yylval_token (TOK_INTCON); }
{IDENT}         { return yylval_token (TOK_IDENT); }

{NOTIDENT}      { scanner_badtoken (yytext); }
{NOTCHAR}       { scanner_badchar (yytext[0]); }
{NOTSTRING}     { scanner_badtoken (yytext); }
.               { scanner_badchar (*yytext); }

%%

RCSC("$Id: scanner.l,v 1.4 2014-11-15 18:37:02-08 - - $")

