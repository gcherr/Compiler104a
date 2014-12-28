
#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "stringset.h"
#include "lyutils.h"
#include "symtab.h"

//struct symbol;

astree* new_astree (int symbol, int filenr, int linenr, int offset,
                    const char* lexinfo) {
   astree* tree = new astree();
   tree->symbol = symbol;
   tree->filenr = filenr;
   tree->linenr = linenr;
   tree->offset = offset;
   tree->lexinfo = intern_stringset (lexinfo);
   tree->attributes = 0;
   //tree->tab_sym = NULL;
   tree->struct_tab_node = new symbol_entry();
   tree->blocknr = 0;
   DEBUGF ('f', "astree %p->{%d:%d.%d: %s: \"%s\"}\n",
           tree, tree->filenr, tree->linenr, tree->offset,
           get_yytname (tree->symbol), tree->lexinfo->c_str());
   return tree;
}


astree* adopt1 (astree* root, astree* child) {
   root->children.push_back (child);
   DEBUGF ('a', "%p (%s) adopting %p (%s)\n",
           root, root->lexinfo->c_str(),
           child, child->lexinfo->c_str());
   return root;
}

astree* adopt2 (astree* root, astree* left, astree* right) {
   adopt1 (root, left);
   adopt1 (root, right);
   return root;
}

astree* adopt1sym (astree* root, astree* child, int symbol) {
   root = adopt1 (root, child);
   root->symbol = symbol;
   return root;
}

astree* adopt2sym (astree* root, astree* child, int symbol) {
   root = adopt1 (root, child);
   child->symbol = symbol;
   return root;
}

astree* changesym(astree* root, int symbol){
   root->symbol = symbol;
   return root;
}

void printsym(astree* root){
  printf("%s\n", get_yytname(root->symbol));
}

void checkblock(astree* root, astree* block){
  if(block->symbol == ';'){
    root->symbol = TOK_PROTOTYPE;
    free_ast(block);
  }
  else{
    adopt1(root, block);
  }
}


static void dump_node (FILE* outfile, astree* node) {
  /*
   fprintf (outfile, "%p->{%s(%d) %ld:%ld.%03ld \"%s\" [",
            node, get_yytname (node->symbol), node->symbol,
            node->filenr, node->linenr, node->offset,
            node->lexinfo->c_str());
  */
  
   const char *tname = get_yytname (node->symbol);
   if(strstr(tname, "TOK_") == tname) tname += 4; 
   fprintf (outfile, "%s \"%s\" %ld.%ld.%ld {%lu} ", tname,
            node->lexinfo->c_str(),node->filenr,
            node->linenr, node->offset, node->blocknr);
   printattr(outfile, node->attributes, node); 
   /*
   bool need_space = false;
   for (size_t child = 0; child < node->children.size(); ++child) {
      if (need_space) fprintf (outfile, " ");
      need_space = true;
      fprintf (outfile, "[%p", node->children.at(child));
   }
   fprintf (outfile, "]}");
   */
}

static void dump_astree_rec (FILE* outfile, astree* root, int depth) {
   if (root == NULL) return;
   //fprintf (outfile, "%*s%s ", depth * 3, "", root->lexinfo->c_str());
   //fprintf (outfile, "%*s", depth * 3, "|");
   for (int iter = 1 ; iter <= depth ; iter++){
     if(root->symbol == TOK_ROOT) continue;
     fprintf (outfile, "%s", "|   ");
   }
   dump_node (outfile, root);
   fprintf (outfile, "\n");
   for (size_t child = 0; child < root->children.size(); ++child) {
      dump_astree_rec (outfile, root->children[child], depth + 1);
   }
}

void dump_astree (FILE* outfile, astree* root) {
   dump_astree_rec (outfile, root, 0);
   fflush (NULL);
}

void yyprint (FILE* outfile, unsigned short toknum, astree* yyvaluep) {
   DEBUGF ('f', "toknum = %d, yyvaluep = %p\n", toknum, yyvaluep);
   if (is_defined_token (toknum)) {
      dump_node (outfile, yyvaluep);
   }else {
      fprintf (outfile, "%s(%d)\n", get_yytname (toknum), toknum);
   }
   fflush (NULL);
}


void free_ast (astree* root) {
   while (not root->children.empty()) {
      astree* child = root->children.back();
      root->children.pop_back();
      free_ast (child);
   }
   DEBUGF ('f', "free [%X]-> %d:%d.%d: %s: \"%s\")\n",
           (uintptr_t) root, root->filenr, root->linenr, root->offset,
           get_yytname (root->symbol), root->lexinfo->c_str());
   delete root;
}

void free_ast2 (astree* tree1, astree* tree2) {
   free_ast (tree1);
   free_ast (tree2);
}

void postorder_assign (astree* root){
  assign_curr_blocknr(root);
  //printf("current element at root is a %s\n",
  //          get_yytname(root->symbol));
  for (unsigned i = 0; i < root->children.size(); ++i){
    
    if(root->children.at(i)->symbol == TOK_FUNCTION){
      visit_function(root->children.at(i));
      continue;
    }
    
    if(root->children.at(i)->symbol == TOK_BLOCK){
      //printf("\nbefore visit block\n");
      visit_block(root->children.at(i));
      //printf("after visit block\n");
      continue;
    }
    postorder_assign(root->children.at(i));
    //printf("%s\n", get_yytname(root->children.at(i)->symbol));
  }
  if(root->symbol != TOK_BLOCK )
     visit_node(root);
  
  //printf("%s\n", get_yytname(root->symbol));
}

void visit_node(astree* root){
  switch(root->symbol){
     case TOK_WHILE: visit_while(root); break;
     case TOK_IF:
     case TOK_IFELSE: visit_if(root); break;
     case TOK_RETURN: break;
       //case '=': visit_decl(root); break;
     case TOK_EQ:
     case TOK_NE:
     case TOK_LT:
     case TOK_LE:
     case TOK_GT:
     case TOK_GE:  visit_compare_ops(root); break;
     case '+':
     case '-':
     case '*':
     case '/':
     case '%': visit_binop_int(root); break;
     case '!': visit_not(root); break;
     case TOK_NEG:
     case TOK_POS: 
        visit_unop_int(root); break;
     case TOK_ORD: 
        visit_ord(root); break;
     case TOK_CHR: 
        visit_chr(root); break;
     case TOK_NEW: break;
     case TOK_FUNCTION: 
        visit_function(root); break;
     case TOK_PROTOTYPE:
     case TOK_IDENT: break;
     case TOK_VARDECL:
        visit_vardecl(root); break;
     case TOK_INTCON: 
        visit_intcon(root); break;
     case TOK_CHARCON: 
        visit_charcon(root); break;
     case TOK_STRINGCON: 
        visit_stringcon(root); break;
     case TOK_FALSE:
     case TOK_TRUE: 
        visit_tf(root); break;
     case TOK_NULL:
        visit_null(root); break;
        break;
     //misc types not in the pdf
     case TOK_ARRAY:
        visit_array(root); break;
     //case TOK_BLOCK:
     //   visit_block(root); break;
     case TOK_STRUCT:
        visit_struct(root); break;
     case TOK_FIELD:
        visit_field(root); break;
     case TOK_INT:
     case TOK_BOOL:
     case TOK_CHAR:
     case TOK_STRING:
        visit_basetypes(root); break;
     case TOK_DECLID:
        visit_declid(root); break;
     case TOK_PARAMLIST:
        visit_paramlist(root); break;
  }
}

//
// Visit functions
//

void visit_vardecl(astree* root){
  root->children.at(0)->children.at(0)->attributes = 
      root->children.at(0)->children.at(0)->attributes | 
      root->children.at(0)->attributes;   
  attr_bitset nulled (string("00000000000000000"));
  attr_bitset check = (root->children.at(0)->attributes & 
                       root->children.at(1)->attributes);
  if(check != nulled){
    root->attributes = check;
     root->children.at(0)->children.at(0)->attributes = 
       root->children.at(0)->children.at(0)->attributes | 
       check;
     declid_actions(root->children.at(0)->children.at(0));

  }else{
    //syserrprintf("Types do not match");
  }

}

void visit_decl(astree* root){
  root = root;
  //printf("inside visit_decl\n");
  if(!get_compatibility(root->children.at(0), 
                        root->children.at(1))){
    //syserrprintf("Types for = operator not compatible");
  }
  //printf("types compatible\n");
}

void visit_while(astree* root){
  if( ! (root->children.at(0)->attributes[ATTR_bool])){
    //syserrprintf("While statement does not use boolean");
  }
}

void visit_if(astree* root){
  if( !(root->children.at(0)->attributes[ATTR_bool])){
    //syserrprintf("If/else statement does not use boolean");
  }
}

void visit_compare_ops(astree* root){
  if(check_children_primitive(root)){
      root->attributes[ATTR_bool] = 1;
      root->attributes[ATTR_vreg] = 1;
    }else{
    //syserrprintf("Comparison type(s) non-primitive"); 
  }
}

void visit_binop_int(astree* root){
  //root = root;
  if(check_children_binop(ATTR_int, root)){
    root->attributes[ATTR_int] = 1;
    root->attributes[ATTR_vreg] = 1;
  }else{
    //syserrprintf("Incompatible type(s) for operator"); 
  }
}

void visit_unop_int(astree* root){
  //printf("%lu", root->children.size());
  if((root->symbol == TOK_POS || root->symbol == TOK_NEG)){
    root->attributes[ATTR_int] = 1;
    root->attributes[ATTR_vreg] = 1;
  }
}

void visit_ord(astree* root){
  if(root->children.at(0)->attributes[ATTR_char]){
     root->attributes[ATTR_int] = 1;
     root->attributes[ATTR_vreg] = 1;
  }else{
    //syserrprintf("Incorrect type for 'ord'");
  }
}

void visit_chr(astree* root){
  if(root->children.at(0)->attributes[ATTR_int]){
     root->attributes[ATTR_char] = 1;
     root->attributes[ATTR_vreg] = 1;
  }else{
    //syserrprintf("Incorrect type for 'chr'");
  }
}

void visit_function(astree* root){
  root->attributes[ATTR_function] = 1;
  //printf("--------------------declid is a %s\n", 
  //       get_yytname(root->children.at(0)->symbol));
  postorder_assign(root->children.at(0));
  //printf("--------------------paramlist is a %s\n",
  //       get_yytname(root->children.at(1)->symbol));
  postorder_assign(root->children.at(1));
  inherit_attr(root, root->children.at(0));
  function_ops(root);
  visit_block(root->children.at(2));
  
}

void visit_not(astree* root){
  if(root->children.at(0)->attributes[ATTR_bool]){
     root->attributes[ATTR_bool] = 1;
     root->attributes[ATTR_vreg] = 1;
  }else{
    //syserrprintf("Incompatible type for boolean"); 
  }
}

void visit_intcon(astree* root){ 
  if(root->symbol == TOK_INTCON){
    root->attributes[ATTR_int] = 1;
    root->attributes[ATTR_const] = 1;
  }else{
    //syserrprintf("Type not int"); 
  }
}

void visit_charcon(astree* root){
   if(root->symbol == TOK_CHARCON){
    root->attributes[ATTR_char] = 1;
    root->attributes[ATTR_const] = 1;
  }
}

void visit_stringcon(astree* root){
   if(root->symbol == TOK_STRINGCON){
    root->attributes[ATTR_string] = 1;
    root->attributes[ATTR_const] = 1;
  }
}

void visit_tf(astree* root){
  if(root->symbol == TOK_TRUE){
    root->attributes[ATTR_bool] = 1;
    root->attributes[ATTR_const] = 1;
  }else if(root->symbol == TOK_FALSE){
    root->attributes[ATTR_bool] = 1;
    root->attributes[ATTR_const] = 1;
  }
}

void visit_null(astree* root){
  if(root->symbol == TOK_NULL){
    root->attributes[ATTR_null] = 1;
    root->attributes[ATTR_const] = 1;
  }
}

void visit_array(astree* root){
  if(root->symbol == TOK_ARRAY){
    root->attributes[ATTR_array] = 1;
  }
}

void visit_block(astree* root){
  //printf("inside block\n");
  enter_block();
  postorder_assign(root);
  /*
  for (unsigned i = 0; i < root->children.size(); ++i){
    postorder_assign(root->children.at(i));
    //block_traverse(root);
  }
  */
  exit_block();
}

void visit_struct(astree* root){
  root = root;
  root->children.at(0)->attributes[ATTR_struct] = 1;
  root->children.at(0)->attributes[ATTR_typeid] = 1;
  insert_to_structtable(root);
  //insert_struct_fields(root);
}

void visit_field(astree* root){
  root->attributes[ATTR_field] = 1;
}

void visit_basetypes(astree* root){
  switch(root->symbol){
     case TOK_BOOL:   root->attributes[ATTR_bool] = 1; 
                      break;
     case TOK_INT:    root->attributes[ATTR_int] = 1; 
                      break;
     case TOK_CHAR:   root->attributes[ATTR_char] = 1; 
                      break;
     case TOK_STRING: root->attributes[ATTR_string] = 1; 
                      break;
  }
}

void visit_declid(astree* root){
   root->attributes[ATTR_lval] = 1;
   root->attributes[ATTR_variable] = 1;
   //declid_actions(root);
}

void visit_paramlist(astree* root){
  for(unsigned i = 0; i < root->children.size(); ++i){
    astree* declnode = root->children.at(i)->children.at(0);
    declnode->blocknr = 1;
    declnode->attributes[ATTR_param] = 1;   
    declnode->attributes = 
      declnode->attributes | root->children.at(0)->attributes;   
  }
}
 
//
//check functions
//

//checks children if they fulfill the types for a binary operator
bool check_children_binop(int attr, astree* root){
  if(root->children.size() < 2){
    return false;
  }
   for (unsigned i = 0; i < root->children.size(); i++){
     //printf("%s\n", get_yytname(root->children.at(i)->symbol));
     //printf("%s\n", get_yytname(chk_token));
     if(!( root->children.at(i)->attributes[attr]) )
       return false;
   }
   return true;  
}

//checks the children of a node if they are primitive types
bool check_children_primitive(astree* root){
   for (unsigned i = 0; i < root->children.size(); i++){
     //printattr(stdout, root->children.at(i)->attributes);
     // printf("\n");
     //printf("%s\n", get_yytname(root->children.at(i)->symbol));
     //visit_node(root->children.at(i));
     if(! check_node_primitive(root->children.at(i)))  {
       return false;
       }
   }
   return true;  
}

//checks a node if it is a primitive type
bool check_node_primitive(astree* root){
   if(!( root->attributes[ATTR_bool]) &&
      !( root->attributes[ATTR_char]) &&
      !( root->attributes[ATTR_int])){
      return false;
   }
   return true;
}

//checks the children of a node if they are reference types
bool check_children_reference(astree* root){
   for (unsigned i = 0; i < root->children.size(); i++){
     if( ! check_node_reference(root->children.at(i))){
       return false;
     }
   }
   return true;
}


//checks node if it is a reference type
bool check_node_reference(astree* root){
  if(root->attributes[ATTR_null] ||
     root->attributes[ATTR_string]){
     return true;
  }

  if(root->attributes[ATTR_struct]){
    if(root->children.at(0)->attributes[ATTR_typeid])
      return true;
  }

  if(root->attributes[ATTR_array]){
    if(root->children.at(0)->attributes[ATTR_bool] ||
       root->children.at(0)->attributes[ATTR_char] ||
       root->children.at(0)->attributes[ATTR_int]  ||
       root->children.at(0)->attributes[ATTR_string] ||
       root->children.at(0)->attributes[ATTR_typeid]){
       return true;
     }
  }

  return false;

}

//checks a child of a node to check for TOK_FIELD
//to be used only with structs
bool check_struct_children_field(astree* root){
  for(unsigned i = 1; i < root->children.size(); i++){
    if(root->children.at(i)->symbol == TOK_FIELD){
      return true;
    }
  }
  return false;
}

//checks compatibility for get operation (=)
bool get_compatibility(astree* leftchild, astree* rightchild){
  bool leftarr = leftchild->attributes[ATTR_array];
  bool rightarr = rightchild->attributes[ATTR_array];
  
  if( (leftarr && rightarr) ){
    if(get_typecheck(leftchild, rightchild)){
      //printf("compatible by typecheck\n");
       return true;
     }else if(check_node_reference(leftchild) && 
              rightchild->symbol == TOK_NULL){
      //printf("compatible by null\n");
       return true;
     }
  }
  //printf("not compatible\n");
  return false;
}

//typecheck for the get operator (=)
bool get_typecheck(astree* leftchild, astree* rightchild){
  if(leftchild->symbol == TOK_ARRAY){
    if(leftchild->children.at(0)->symbol == 
       rightchild->children.at(0)->symbol){
       return true;
    }else{
       return false;
    }
  }else{
    if(leftchild->symbol == rightchild->symbol)
      return true;
    else
     return false;
  }
}

void find_node(astree* root, symbol_table sym, FILE* outfile){
      const auto& entry = sym.find (root->lexinfo);
      outfile = outfile;
      if (entry == sym.end()){
        //syserrprintf("Variable not declared");
      } 
      // else
      //symbol_entry sym = make_pair(entry->first,entry->second);
}

//
//print helper
//
void printattr(FILE* outfile, attr_bitset bits, astree* node){
  if(bits[ATTR_void] == 1){
    fprintf(outfile, "void ");
  }
  if(bits[ATTR_bool] == 1){
    fprintf(outfile, "bool ");
  }
  if(bits[ATTR_char] == 1){
    fprintf(outfile, "char ");
  }
  if(bits[ATTR_int] == 1){
    fprintf(outfile, "int ");
  }
  if(bits[ATTR_null] == 1){
    fprintf(outfile, "null ");
  }
  if(bits[ATTR_string] == 1){
    fprintf(outfile, "string ");
  }
  if(bits[ATTR_struct] == 1){
    fprintf(outfile, "struct ");
  }
  if(bits[ATTR_array] == 1){
    fprintf(outfile, "array ");
  }
  if(bits[ATTR_function] == 1){
    fprintf(outfile, "function ");
  }
  if(bits[ATTR_variable] == 1){
    fprintf(outfile, "variable ");
  }
  if(bits[ATTR_field] == 1){
    //fprintf(outfile, "field ");
  }
  if(bits[ATTR_typeid] == 1){
    fprintf(outfile, "\"%s\" ",node->lexinfo->c_str());
  }
  if(bits[ATTR_param] == 1){
    fprintf(outfile, "param ");
  }
  if(bits[ATTR_lval] == 1){
    fprintf(outfile, "lval ");
  }
  if(bits[ATTR_const] == 1){
    fprintf(outfile, "const ");
  }
  if(bits[ATTR_vreg] == 1){
    fprintf(outfile, "vreg ");
  }
  if(bits[ATTR_vaddr] == 1){
    fprintf(outfile, "vaddr ");
  }
}

RCSC("$Id: astree.cc,v 1.4 2014-12-01 16:34:53-08 - - $")

