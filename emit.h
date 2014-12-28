#ifndef __EMIT_H__
#define __EMIT_H__

#include "astree.h"

extern FILE* oilfile;

void emit_sm_code (astree*);
void emit_insn (const char* operand, astree* tree, string dest);
void emit_newline();
void postorder (astree* tree);
void emit_function(astree* tree);
void emit_params(astree* tree);
void emit_struct(astree* tree);
void emit_fielddecl(astree* tree);
void emit_vardecl(astree* tree);
void emit_vardecl_old (astree* tree);
void blocknr_prefix(astree* tree);
void postorder_emit_stmts (astree* tree);
void postorder_emit_binop (astree* tree);
void postorder_emit_unop (astree* tree);
void emit_semi (astree* tree);
void emit_push (astree* tree);
void emit_assign (astree* tree);

void emit (astree* tree);
void emit_sm_code (astree* tree);
void emit_global_string(FILE* oilfile);
void emit_oc_string(FILE* oilfile);
void emit_struct_string(FILE* oilfile);
void emit_stringcon_string(FILE* file);
void emit_func_string(FILE* file);
void switch_current_string (string s);
void point_to_string (string s);

RCSH("$Id: emit.h,v 1.1 2013-09-19 16:38:25-07 - - $")
#endif
