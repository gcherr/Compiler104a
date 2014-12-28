#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include <bitset>
#include <typeinfo>

#include "auxlib.h"
#include "astree.h"
#include "symtab.h"

int next_block = 1;
int global_block_count = 0;
vector<int> block_stack;
vector<symbol_table*> symbol_stack;
symbol_table global_structs;
symbol_table global_idents;
//FILE* symfile;
//symbol_table* fieldtable = new symbol_table();

symbol* new_symbol(attr_bitset attributes, symbol_table* fields,
                   size_t filenr, size_t linenr, size_t offset,
                   size_t block_nr, vector<symbol*>* parameters){
  symbol* sym = new symbol();
  sym->attributes = attributes;
  sym->fields = fields;
  sym->filenr = filenr;
  sym->linenr = linenr;
  sym->offset = offset;
  sym->block_nr = block_nr;
  sym->parameters = parameters;
  return sym; 
}

void enter_block(){
  ++global_block_count;
  //printf("global block count : %d\n", global_block_count);
  symbol_stack.push_back(nullptr);
  //printf("entering size of the symbol_stack: %lu\n",
  //          symbol_stack.size());
  block_stack.push_back(global_block_count); 
  /*
  printf("block stack pushed, size: %lu, top element: %d \n",
         block_stack.size(), block_stack.at(block_stack.size()-1));
  printf("block stack elements after enter:");
  for(unsigned i = 0; i < block_stack.size(); i++){
    printf("%d, ", block_stack.at(i));
  }
  printf("\n");
  */
}

void exit_block(){
  block_stack.pop_back();
  /*
  printf("exiting size of the block stack: %lu\n", block_stack.size());
   printf("block stack elements after exit:");
  for(unsigned i = 0; i < block_stack.size(); i++){
    printf("%d, ", block_stack.at(i));
  }
  printf("\n");
  printf("exiting size of the symbol_stack before pop_back: %lu\n",
          symbol_stack.size());
  printf("\ntable at top of symbol stack:\n");
  if(symbol_stack.size() == 0){
     printf("No table at top of symbol stack\n");
  }
  else if((symbol_stack.at(symbol_stack.size()-1)) != nullptr){
    
     print_symbol_table(stdout, 
                      *(symbol_stack.at(symbol_stack.size()-1)));
  }else{
    printf("\n***empty table***\n\n");
  }
  */
  if(symbol_stack.size() != 0){
     symbol_stack.pop_back();
  }
  // printf("exiting size of symbol_stack after 1st pop_back: %lu\n", 
  //        symbol_stack.size());
  //while((symbol_stack.at(symbol_stack.size()-1)) == nullptr &&
  //      symbol_stack.size() != 0){
  while(symbol_stack.size() != 0){
    if((symbol_stack.at(symbol_stack.size()-1)) != nullptr)
      break;
    //  printf("popping nullptr from top of symbol_stack\n");
    symbol_stack.pop_back();
    //  printf("size of symbol stack during repeated pop:%lu\n",
    //          symbol_stack.size());
  }
  //printf("exiting size of the symbol_stack after pop_back: %lu\n", 
  //        symbol_stack.size());
  fprintf(symfile, "\n");
}

void block_traverse(astree* root){
  for (unsigned i = 0; i < root->children.size(); ++i){
    postorder_assign(root->children.at(i));
  }
  visit_node(root);
}

void func_traverse(astree* root){
  for (unsigned i = 0; i < root->children.size(); ++i){
    postorder_assign(root->children.at(i));
  }
  visit_node(root);
}

void declid_actions(astree* root){
  //printf("\nfound declid : %s\n", root->lexinfo->c_str());
  if(symbol_stack.size() == 0){
     symbol_table* block_table = new symbol_table();
     symbol_stack.push_back(block_table);
     // printf("No current symbol table, insert a new one\n");
  }
  else if(symbol_stack.at(symbol_stack.size()-1) == nullptr){
    symbol_table* block_table = new symbol_table();
    symbol_stack.push_back(block_table);
    // printf("Found nullptr, NEW symtab inserted on top of stack\n");
  }

  //printf("test1\n");
  
  if(!check_dup_var(root)){   
    return;
  }
  
  //printf("test2\n");
  symbol* decl = new_symbol(root->attributes,
                          nullptr, root->filenr, 
                          root->linenr, root->offset,
                          block_stack.at(block_stack.size()-1),
                          nullptr);
  root->blocknr = decl->block_nr;
  //printf("test3\n");
  symbol_table* sym_stack_top = symbol_stack.at(symbol_stack.size()-1);
  //printf("test4\n");
  if(sym_stack_top == nullptr){
    printf("sym_stack_top == null\n");
  }
  //printf("test5\n");
  (*sym_stack_top).insert(symbol_entry(root->lexinfo, decl));
  //printf("size of sym_stack_top = %lu\n", (*sym_stack_top).size());
  //printf("test6\n");
  printtosym(root, symfile, block_stack.at(block_stack.size()-1)); 
}

bool check_dup_var(astree* root){
  bool no_dup = true;
  //for(unsigned i = 0; i < symbol_stack.size(); i++){
  //printf("assigning a symbol table to 's'\n");
    //if(symbol_stack.at(i) == nullptr){
    if(symbol_stack.at(symbol_stack.size()-1) == nullptr){
      //printf("stack element null\n");
      //continue;
      return no_dup;
    }
    //symbol_table s = *symbol_stack.at(i);
    symbol_table s = *symbol_stack.at(symbol_stack.size()-1);
    if(s.size() == 0){
      //printf("size 0 for symbol stack element\n");
      //continue;
      return no_dup;
    }
    //printf("iterating through stack elements\n");
     for(auto iter = s.begin(); iter != s.end(); iter++){
       if(root->lexinfo == iter->first){
         no_dup = false;
         //syserrprintf("Duplicate variable declaration");
       }     
     }
  //}
  return no_dup;
}

void block_traverse_old(astree* root){
  //printf("%lu\n", root->children.size());
  symbol* decl = nullptr;
  if(root->symbol == TOK_DECLID){
     //printf("found declid\n");
     symbol* decl = new_symbol(root->attributes,
                             nullptr, root->filenr, 
                             root->linenr, root->offset,
                             block_stack.at(block_stack.size()-1),
                             nullptr);
     root->blocknr = decl->block_nr;
     global_idents.insert(symbol_entry(root->lexinfo, decl));
     printtosym(root, symfile, block_stack.at(block_stack.size()-1)); 
  }
  //for(unsigned i = root->children.size(); i > 0; --i){
  for(unsigned i = 0; i < root->children.size(); i++){
    //print_block_elem(root->children.at(i));
    //printf("in block_traverse\n");
    /*
    if(root->symbol == TOK_DECLID){
      //printf("found declid\n");
      symbol* decl = new_symbol(root->children.at(i)->attributes,
                             nullptr, root->filenr, 
                             root->linenr, root->offset,
                             (size_t)global_block_count, nullptr);
      global_idents.insert(symbol_entry(root->lexinfo, decl));
      printtosym(root, stdout, block_stack.size()); 
    }
    */
    //if(root->children.at(i)->symbol == TOK_BLOCK) continue;
    block_traverse(root->children.at(i));
  }
  decl = decl;
}



/*
void insert_symbol(astree* root){
  symbol* structsym = new symbol(root->attributes,
                                fieldtab,root->filenr,
                                root->linenr, root->offset,
                                0,NULL);
  global_structs.insert(root->lexinfo, structsym);  
  
}
*/

void insert_to_structtable_old(astree* root){
  symbol_table* fieldtable = new symbol_table();
  //fieldtable = nullptr; //old version
  symbol* structsym = new_symbol(root->attributes,
                                fieldtable,root->filenr,
                                root->linenr, root->offset,
                                0,nullptr);
  const string* typeidlex = root->children.at(0)->lexinfo;

  global_structs.insert(symbol_entry(typeidlex,structsym));

  //prints the child(typeid) lexinfo
  printtosym(root->children.at(0), symfile, 0);

  //insert the structs children(fields) into field table
  insert_struct_fields_old(root, structsym);

  /*
  for(int i = 0; i < 5; i++){
     symbol* test = new_symbol(root->attributes,
                                nullptr,9,
                                9, 9,
                                0,nullptr);
     string* a =(string*) "blah";
     structsym->fields->insert(make_pair(a, test));
   }
  */
  
  //print_symbol_table_names(stdout, *fieldtable);
  fprintf(symfile, "\n");
}

void insert_to_structtable(astree* root){
  symbol_table* fieldtable = new symbol_table();
  symbol* structsym = new_symbol(root->attributes,
                                fieldtable,root->filenr,
                                root->linenr, root->offset,
                                0,nullptr);
  const string* typeidlex = root->children.at(0)->lexinfo;
  global_structs.insert(symbol_entry(typeidlex,structsym));
  printtosym(root->children.at(0), symfile, 0);
  insert_struct_fields(root, fieldtable);
  //print_symbol_table(stdout, *fieldtable);
  fprintf(symfile, "\n");
}

void insert_struct_fields(astree* root, symbol_table* sym){
  if(root->children.size() > 1){
    for(unsigned i = 1; i < root->children.size(); i++){
      astree* currchild = root->children.at(i);
      astree* fieldchild = nullptr;
      if(currchild->children.at(0)->attributes[ATTR_field]){
         fieldchild = currchild->children.at(0);
      }else{
        fieldchild = currchild->children.at(1);
      }
      inherit_attr(fieldchild,currchild);
      const string* fieldlex = fieldchild->lexinfo;
      symbol* fieldsym = new_symbol(fieldchild->attributes,
                 nullptr, fieldchild->filenr,fieldchild->linenr, 
                 fieldchild->offset, 0, nullptr);     
      //print_symbol(stdout, fieldsym);  

      (*sym).insert(symbol_entry(fieldlex, fieldsym));
      printfields(fieldchild, symfile,
                     root->children.at(0)->lexinfo->c_str());
    }
  }
  //print_symbol_table(stdout, *sym);
}

//handles struct fields
//the fields variable in this case refers to the fieldtable
void insert_struct_fields_old(astree* root,symbol* sym){
  if(root->children.size() > 1){
    symbol_table fieldtab;
    /* symbol_table* tabptr;
    tabptr = &fieldtab;
    fieldtable = tabptr;*/
     for(unsigned i = 1; i < root->children.size(); i++){
         astree* currchild = root->children.at(i);
         astree* fieldchild = nullptr;
         if(currchild->children.at(0)->attributes[ATTR_field]){
            fieldchild = currchild->children.at(0);
         }else{
           fieldchild = currchild->children.at(1);
         }
         inherit_attr(fieldchild, currchild);
         const string* fieldlex = fieldchild->lexinfo;
         symbol* fieldsym = new_symbol(fieldchild->attributes,
                 nullptr, fieldchild->filenr,fieldchild->linenr, 
                 fieldchild->offset, 0, nullptr);     
         //print_symbol(stdout, fieldsym);  

         fieldtab.insert(symbol_entry(fieldlex, fieldsym));
    
         //symbol_entry sym_in = make_pair(fieldlex, fieldsym);
         //sym->fields->insert(sym_in);

         //not inserting correctly
         //print_symbol_table(stdout, fieldtab);
         printfields(fieldchild, symfile,
                     root->children.at(0)->lexinfo->c_str());
         //printtosym(fieldchild, symfile, 1);
     }
     //print_symbol_table(stdout, fieldtab); 
      sym->fields = &fieldtab;
      print_symbol_table(stdout, *sym->fields);
  }

}

void inherit_attr(astree* target, astree* source){
  if(source->attributes[ATTR_array]){
    target->attributes =(target->attributes |
                source->children.at(0)->attributes);
  }
   target->attributes =(target->attributes|source->attributes);
}

void assign_curr_blocknr(astree* root){
  if(block_stack.size() == 0)
    root->blocknr = 0;
  else
    root->blocknr = block_stack.at(block_stack.size()-1);
}

void function_ops(astree* root){
  vector<symbol*>* params = new vector<symbol*> ();
  assign_curr_blocknr(root);
  printtosym(root,symfile, root->blocknr);
  astree* declnode = root->children.at(0)->children.at(0);
  symbol* funcsym = new_symbol(declnode->attributes,
                                nullptr,declnode->filenr,
                                declnode->linenr, declnode->offset,
                                0,params);
  assign_params(root, params);
  global_idents.insert(symbol_entry(declnode->lexinfo,funcsym));
}

void assign_params(astree* root, vector<symbol*>* par){
  astree* paramnode = root->children.at(1);
  for(unsigned i = 0 ; i < paramnode->children.size(); i++){
    astree* declnode = paramnode->children.at(i)->children.at(0);
    symbol* paramsym = new_symbol(declnode->attributes,
                                nullptr,declnode->filenr,
                                declnode->linenr, declnode->offset,
                                declnode->blocknr,nullptr);
    (*par).push_back(paramsym);
    printtosym(declnode, symfile, declnode->blocknr);
  }
  fprintf(symfile, "\n");
  //root->parameters.push_back(paramsym);
  
}

void print_block_elem(astree* root){
  root=root;
}

void print_symbol_table(FILE* outfile, symbol_table s){
     for(auto iter = s.begin(); iter != s.end(); iter++){
       fprintf(outfile, "*********************\n");
       fprintf(outfile,"KEY:%s\n", iter->first->c_str());       
       print_symbol(outfile, iter->second);
       //fprintf(outfile, "*********************\n");
     }
}

void print_symbol_table_names(FILE* outfile, symbol_table s){
    for(auto iter = s.begin(); iter != s.end(); iter++){
       fprintf(outfile,"KEY:%s\n", iter->first->c_str());
       fprintf(outfile, "*********************\n");
       //print_symbol(outfile, iter->second);
       //fprintf(outfile, "*********************\n");
     }
}

void print_symbol(FILE* outfile, symbol* s){
  fprintf(outfile, "---SYMBOL---\n");
  fprintf(outfile, "ATTRIBUTES:");
  printattr(outfile, s->attributes, nullptr);

  fprintf(outfile, "\n");
  
  fprintf(outfile," (%ld.%ld.%ld) {%lu} \n",
       s->filenr, s->linenr, s->offset, s->block_nr);
  fprintf(outfile, "\n");
  //symbol_table* temp = s->fields;
  if(s->fields != NULL){
    fprintf(outfile, "+++++++++++++FIELDS+++++++++++++ \n");
    print_symbol_table(outfile, *s->fields);
    fprintf(outfile, "++++++++++++++++++++++++++++++++ \n");
  }
  else
     fprintf(outfile, "Field Table is NULL\n");
  if(s->parameters != NULL){
    fprintf(outfile, "+++++++++++++PARAMS+++++++++++++ \n");
    //print_symbol_table(outfile, *s->fields);
    vector<symbol*> symvec = *s->parameters;
    for(unsigned i = 0; i < (symvec).size(); i++){
      print_symbol(outfile, symvec.at(i));
    }
    fprintf(outfile, "++++++++++++++++++++++++++++++++ \n");
  }
  else
     fprintf(outfile, "Param Table is NULL\n");
}

void printtosym(astree* node, FILE* outfile, int indent){
  for (int iter = 1 ; iter <= indent ; iter++){
     fprintf (outfile, "%s", "   ");
   }
  if(node->symbol != TOK_FUNCTION){
    fprintf(outfile, "%s (%ld.%ld.%ld) ", 
            node->lexinfo->c_str(),node->filenr,
            node->linenr, node->offset);
  }
  else{
    astree* declnode = node->children.at(0)->children.at(0);
    fprintf(outfile, "%s (%ld.%ld.%ld) ",
            declnode->lexinfo->c_str(),declnode->filenr,
            declnode->linenr, declnode->offset);
  }
  if(!node->attributes[ATTR_field]){
    fprintf(outfile, "{%lu} ", node->blocknr);
  }
  printattr(outfile, node->attributes, node);
  fprintf(outfile,"\n");
}

void printfields(astree* node, FILE* outfile, const char* str){
   fprintf (outfile, "%s", "   ");
   fprintf(outfile, "%s (%ld.%ld.%ld) ", 
          node->lexinfo->c_str(),node->filenr,
          node->linenr, node->offset);
   fprintf(outfile, "field {%s} ",str );
   
   printattr(outfile, node->attributes, node);
   fprintf(outfile,"\n");
}
