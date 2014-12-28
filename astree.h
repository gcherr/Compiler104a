#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
#include <bitset>
using namespace std;

#include "auxlib.h"
#include "symtab.h"

//struct symbol;

struct astree {
   int symbol;               // token code
   size_t filenr;            // index into filename stack
   size_t linenr;            // line number from source code
   size_t offset;            // offset of token with current line
   const string* lexinfo;    // pointer to lexical information
   attr_bitset attributes;
   size_t blocknr;
   symbol_entry* struct_tab_node;
  //symbol* tab_sym;
   vector<astree*> children; // children of this n-way node
};


astree* new_astree (int symbol, int filenr, int linenr, int offset,
                    const char* lexinfo);
astree* adopt1 (astree* root, astree* child);
astree* adopt2 (astree* root, astree* left, astree* right);
astree* adopt1sym (astree* root, astree* child, int symbol);
astree* adopt2sym (astree* root, astree* child, int symbol);
astree* changesym(astree* root, int symbol);
void checkblock(astree* root, astree* block);
void printsym(astree* root);
void dump_astree (FILE* outfile, astree* root);
void yyprint (FILE* outfile, unsigned short toknum, astree* yyvaluep);
void free_ast (astree* tree);
void free_ast2 (astree* tree1, astree* tree2);
void postorder_assign(astree* ast);

void visit_node(astree* root);
void visit_vardecl(astree* root);
void visit_decl(astree* root);
void visit_while(astree* root);
void visit_if(astree* root);
void visit_compare_ops(astree* root);
void visit_binop_int(astree* root);
void visit_unop_int(astree* root);
void visit_ord(astree* root);
void visit_chr(astree* root);
void visit_function(astree* root);
void visit_not(astree* root);
void visit_intcon(astree* root);
void visit_charcon(astree* root);
void visit_stringcon(astree* root);
void visit_tf(astree* root);
void visit_declid(astree* root);

void visit_null(astree* root);
void visit_array(astree* root);
void visit_block(astree* root);
void visit_struct(astree* root);
void visit_field(astree* root);
void visit_basetypes(astree* root);
void visit_paramlist(astree* root);

bool check_children_binop(int attr, astree* root);
bool check_children_primitive(astree* root);
bool check_node_primitive(astree* root);
bool check_child_reference(astree* root);
bool check_node_reference(astree* root);
bool check_struct_children_field(astree* root); 
bool get_compatibility(astree* leftchild, astree* rightchild);
bool get_typecheck(astree* leftchild, astree* rightchild);

void find_node(astree* root, symbol_table sym);

void printattr(FILE* outfile, attr_bitset bits, astree* node);


RCSH("$Id: astree.h,v 1.4 2014-12-01 16:34:53-08 - - $")
#endif
