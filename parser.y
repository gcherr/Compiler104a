%{
// $Id: parser.y,v 1.5 2014-12-01 16:34:53-08 - - $

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "lyutils.h"
#include "astree.h"

#define YYDEBUG 1
#define YYERROR_VERBOSE 1
#define YYPRINT yyprint
#define YYMALLOC yycalloc

static void* yycalloc (size_t size);

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%token TOK_VOID TOK_BOOL TOK_CHAR TOK_INT TOK_STRING
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_FALSE TOK_TRUE TOK_NULL TOK_NEW TOK_ARRAY
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON

%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_INITDECL
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token TOK_ORD TOK_CHR TOK_ROOT

%token TOK_DECLID TOK TOK_PARAMLIST TOK_FUNCTION
%token TOK_PROTOTYPE TOK_RETURNVOID TOK_VARDECL
%token TOK_NEWSTRING TOK_INDEX

%right    TOK_IF TOK_ELSE
%right    '='
%left     TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left     '+' '-'
%left     '*' '/' '%'
%right    TOK_POS TOK_NEG '!' TOK_ORD TOK_CHR
%left     '[' '.' TOK_CALL
%nonassoc TOK_NEW
%nonassoc '('

%start  start


%%

start     : program               { yyparse_astree = $1; }
          ;

program   : program structdef     { $$ = adopt1 ($1, $2); }
          | program function      { $$ = adopt1 ($1, $2); }
          | program statement     { $$ = adopt1 ($1, $2); }
          | program error '}'     { $$ = $1; }
          | program error ';'     { $$ = $1; }
          |                       { $$ = new_parseroot(); }
          ;

structdef : structroot '}'        { free_ast($2); $$ = $1; }
          ;

structroot: structroot fielddecl ';'
                                  { free_ast($3); $$ = adopt1($1,$2); }
          | TOK_STRUCT TOK_IDENT '{' 
                                  { free_ast($3); 
                                    $1->offset = $2->offset;
                                    $$ = adopt2sym($1,$2,TOK_TYPEID); }
          ;

function  : identdecl '(' ')' block    
                             { free_ast($3); 
                               astree* root = new_astree(TOK_FUNCTION,
                               $1->filenr,$1->linenr,$1->offset,"");
                               $$ = adopt1(root,$1);
                               $$ = adopt2sym(root,$2,TOK_PARAMLIST); 
                               checkblock(root, $4); }
          | identdecl funcroot ')' block
                             { free_ast($3); 
                               astree* root = new_astree(TOK_FUNCTION,
                               $1->filenr,$1->linenr,$1->offset,"");
                               $$ = adopt2(root,$1,$2); 
                               checkblock(root, $4); }
          ;

funcroot  : funcroot ',' identdecl
                                  { free_ast($2); adopt1($1,$3); }
          | '(' identdecl
                             { $$ = adopt1sym($1,$2,TOK_PARAMLIST); }
          ;

identdecl : basetype TOK_IDENT    { $$ = adopt2sym ($1,$2,TOK_DECLID);}
          | basetype TOK_NEWARRAY TOK_IDENT
                             { $$ = adopt2 (changesym($2, TOK_ARRAY),
                               $1, changesym($3, TOK_DECLID));}
          ;

block     : blockroot '}'         { free_ast($2); $$ = $1; }
          | ';'                   { $$ = $1; }
          ;
 
blockroot : blockroot statement   { $$ = adopt1($1, $2); }
          | '{'                   { $$ = changesym($1, TOK_BLOCK);}
          ;

statement : block                 { $$ = $1; }
          | vardecl               { $$ = $1; }
          | while                 { $$ = $1; }
          | ifelse                { $$ = $1; }
          | return                { $$ = $1; }
          | expr ';'              { free_ast($2); $$ = $1; }
          ;

vardecl   : identdecl '=' expr ';'
                                  { free_ast($4),
                                    $$ = adopt1sym($2,$1, TOK_VARDECL);
                                    $$ = adopt1($2,$3); }
          ;

while     : TOK_WHILE '(' expr ')' statement
                                  { free_ast2($2,$4);
                                    $$ = adopt2($1,$3,$5); }
          ;

ifelse    : TOK_IF '(' expr ')' statement %prec TOK_ELSE
                                  { free_ast2($2,$4);
                                    adopt2($1, $3, $5); }
          | TOK_IF '(' expr ')' statement TOK_ELSE statement
                                  { free_ast2($2,$4); free_ast($6);
                                    adopt1sym($1, $3, TOK_IFELSE);
                                    adopt2($1, $5, $7); }
          ;

fielddecl : basetype TOK_IDENT    { $$ = adopt2sym ($1,$2,TOK_FIELD);}
          | basetype TOK_NEWARRAY TOK_IDENT
                                  { $$=adopt2(changesym($2,TOK_ARRAY),
                                    $1, changesym($3, TOK_FIELD));}
          ;

basetype  : TOK_VOID              { $$ = $1; }
          | TOK_BOOL              { $$ = $1; }
          | TOK_CHAR              { $$ = $1; }
          | TOK_INT               { $$ = $1; }
          | TOK_STRING            { $$ = $1; }
          | TOK_IDENT             { $$=changesym($1, TOK_TYPEID);}
          ;

return    : TOK_RETURN ';'        { $$=changesym($1,TOK_RETURNVOID);}
          | TOK_RETURN expr ';'   { free_ast($3);
                                    $$ = adopt1($1,$2);}
          ;

expr      : expr '=' expr         { $$ = adopt2 ($2, $1, $3); }
          | expr TOK_EQ expr      { $$ = adopt2 ($2, $1, $3); }
          | expr TOK_NE expr      { $$ = adopt2 ($2, $1, $3); }
          | expr TOK_LT expr      { $$ = adopt2 ($2, $1, $3); }
          | expr TOK_LE expr      { $$ = adopt2 ($2, $1, $3); }
          | expr TOK_GT expr      { $$ = adopt2 ($2, $1, $3); }
          | expr TOK_GE expr      { $$ = adopt2 ($2, $1, $3); }
          | expr '+' expr         { $$ = adopt2 ($2, $1, $3); }
          | expr '-' expr         { $$ = adopt2 ($2, $1, $3); }
          | expr '*' expr         { $$ = adopt2 ($2, $1, $3); }
          | expr '/' expr         { $$ = adopt2 ($2, $1, $3); }
          | expr '%' expr         { $$ = adopt2 ($2, $1, $3); }
          | '+' expr %prec TOK_POS    
                                  { $$ = adopt1sym ($1, $2, TOK_POS); }
          | '-' expr %prec TOK_NEG    
                                  { $$ = adopt1sym ($1, $2, TOK_NEG); }
          | '!' expr              { $$ = adopt1 ($1, $2); }
          | TOK_ORD expr          { $$ = adopt1 ($1, $2); }
          | TOK_CHR expr          { $$ = adopt1 ($1, $2); }
          | allocator             { $$ = $1; }
          | call                  { $$ = $1; }
          | '(' expr ')'     
                                  { free_ast2 ($1, $3); $$ = $2; }
          | variable              { $$ = $1; }
          | constant              { $$ = $1; }
          ;

allocator : TOK_NEW TOK_IDENT '(' ')'
                                  { free_ast2($3,$4);
                                    $$ = adopt2sym($1,$2, TOK_TYPEID); }
          | TOK_NEW TOK_STRING '(' expr ')'
                                  { free_ast($2); free_ast2($3,$5);
                                    $$=adopt1sym($1,$4,TOK_NEWSTRING);}
          | TOK_NEW basetype '[' expr ']'
                                  { $$ = adopt1sym($1,$2, TOK_NEWARRAY);
                                    $$ = adopt1($1, $4); }
          ;

call      : TOK_IDENT '('')'      { free_ast($3); 
                                    $$ = adopt1sym($2, $1, TOK_CALL);}
          | callroot ')'          { free_ast($2); $$ = $1; }
          ;

callroot  : callroot ',' expr     { free_ast($2); adopt1($1,$3); }
          | TOK_IDENT '(' expr    { $$ = adopt2(changesym($2, TOK_CALL),
                                    $1, $3); }

variable  : TOK_IDENT             { $$ = $1; }
          | expr '[' expr ']' %prec '['   
                                  { free_ast($4); 
                                    $$=adopt2(changesym($2,TOK_INDEX),
                                    $1,$3); }
          | expr '.' TOK_IDENT  
                                  { $$ = adopt2($2, $1, 
                                    changesym($3, TOK_FIELD)); } 
          ;

constant  : TOK_INTCON            { $$ = $1; }
          | TOK_CHARCON           { $$ = $1; }
          | TOK_STRINGCON         { $$ = $1; }
          | TOK_FALSE             { $$ = $1; }
          | TOK_TRUE              { $$ = $1; }
          | TOK_NULL              { $$ = $1; }
          ;

%%

const char* get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}

bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}

static void* yycalloc (size_t size) {
   void* result = calloc (1, size);
   assert (result != NULL);
   return result;
}

RCSC("$Id: parser.y,v 1.5 2014-12-01 16:34:53-08 - - $")


