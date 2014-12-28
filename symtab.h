#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <bitset>

using namespace std;

#include "auxlib.h"
#include "astree.h"
#include "lyutils.h"

extern FILE* symfile;

struct astree;

enum { ATTR_void, ATTR_bool, ATTR_char, ATTR_int, ATTR_null, 
ATTR_string, ATTR_struct, ATTR_array, ATTR_function, ATTR_variable,
ATTR_field, ATTR_typeid, ATTR_param, ATTR_lval, ATTR_const,
ATTR_vreg, ATTR_vaddr, ATTR_bitset_size,
};
using attr_bitset = bitset<ATTR_bitset_size>;
//extern attr_biset;

struct symbol;
using symbol_table = unordered_map<const string*,symbol*>;
using symbol_entry = pair<const string*, symbol*>;

extern symbol_table global_idents;
extern symbol_table global_structs;

struct symbol{
  attr_bitset attributes;
  symbol_table* fields;
  size_t filenr, linenr, offset;
  size_t block_nr;
  vector<symbol*>* parameters;
};

symbol* new_symbol(attr_bitset attributes, symbol_table* fields,
                   size_t filenr, size_t linenr, size_t offset,
                   size_t block_nr, vector<symbol*>* parameters);

extern vector<symbol_table*> symbol_stack;
extern vector<int> block_stack;

void enter_block();
void exit_block();

void block_traverse(astree* root);
void func_traverse(astree* root);
void block_traverse_old(astree* root);
void declid_actions(astree* root);
bool check_dup_var(astree* root);
void insert_symbol(astree* root);
void insert_to_structtable(astree* root);
void insert_to_structtable_old(astree* root);
void insert_struct_fields(astree* root, symbol_table* sym);
void insert_struct_fields_old(astree* root, symbol* sym);
void inherit_attr(astree* target, astree* source);
void assign_curr_blocknr(astree* root);
void function_ops(astree* root);
void assign_params(astree* root, vector<symbol*>* par);
/*
void postorder_assign (astree* root);
void visit_node(astree* root);
*/
//extern int next_block;
void print_symbol_table(FILE* outfile, symbol_table s);
void print_symbol_table_names(FILE* outfile, symbol_table s);
void print_symbol(FILE* outfile, symbol* s);
void printtosym(astree* node,  FILE* outfile, int indent);
void printfields(astree* node,  FILE* outfile, const char* str);



#endif
