
#include <stdio.h>
#include <assert.h>


#include "astree.h"
#include "emit.h"
#include "lyutils.h"
#include "auxlib.h"

//string* strptr = new string("stringtest\n");
string globalstring = "void__ocmain (void)\n{\n";
string currstring = "";
string ocstring = "";
string funcstring = "";
string structstring = "";
string stringstring = "";
string eightspaces = "        ";
string teststr = "cat";
string* strptr = &teststr;
bool global = true;

void emit (astree*);

void emit_insn (const char* operand, astree* tree, string dest) {
  /*
  fprintf (stdout, "%-10s%-20s; %s %ld.%ld", "",
    operand, scanner_filename (tree->filenr)->c_str(),
            tree->linenr, tree->offset);
   */
  /*
   fprintf (stdout, "%s",
            operand);
  */
  dest.append(operand);
  tree = tree;
}

void emit_newline(){
  currstring.append("\n");
}

void postorder (astree* tree) {
   assert (tree != NULL);
   for (size_t child = 0; child < tree->children.size(); ++child) {
     //fprintf(stdout, "CHILD: %d\n", tree->children.at(child)->symbol);
      emit (tree->children.at(child));
   }
}

void emit_struct(astree* tree){
  structstring.append("struct s_");
  structstring.append(tree->lexinfo->c_str());
  structstring.append(" {\n");
  for(unsigned i = 1; i < tree->children.size(); i++){
    structstring.append(eightspaces);
    emit_fielddecl(tree->children.at(i));
    structstring.append("\n");
  }
  structstring.append("};\n");
}

void emit_fielddecl(astree* tree){
  if(tree->children.size() == 1){
    structstring.append(tree->lexinfo->c_str());
    structstring.append(" ");
    structstring.append(tree->children.at(0)->
                        lexinfo->c_str());
  }else{
    structstring.append(tree->children.at(0)->
                        lexinfo->c_str());
    structstring.append(tree->lexinfo->c_str());
    structstring.append(" ");
    structstring.append(tree->children.at(1)->
                        lexinfo->c_str());
  }
  structstring.append(";");
}

void emit_function(astree* tree){
  global = false;
  funcstring.append(tree->children.at(0)->lexinfo->c_str());
  funcstring.append(" __");
  funcstring.append(tree->children.at(0)->children.at(0)->
                    lexinfo->c_str());
  funcstring.append(" {\n");
  emit_params(tree->children.at(1));
  funcstring.append("}\n");
}

void emit_params(astree* tree){
  for(unsigned i = 0 ; i < tree->children.size(); i++){
    funcstring.append(eightspaces);
    funcstring.append(tree->children.at(0)->lexinfo->c_str());
    funcstring.append(" _1_");
    //funcstring.append("fsjal");
    funcstring.append(tree->children.at(0)->children.at(0)->
                      lexinfo->c_str());
    funcstring.append(",\n");
  }
  if(tree->children.size() > 0){
    funcstring.append(")\n");
  }
}

void emit_vardecl (astree* tree){
  if(tree->blocknr != 0)
    global = false;
  else
    global = true;
  if(global){
    globalstring.append(eightspaces);
    if(tree->children.at(0)->children.size() == 1){
      blocknr_prefix(tree);
      globalstring.append(tree->children.at(0)->children.at(0)->
                          lexinfo->c_str());
      /*
      emit(tree->children.at(0));
      currstring.append(" ");
      emit(tree->children.at(0)->children.at(0));
      */
    }
    globalstring.append(" ");
    globalstring.append( tree->lexinfo->c_str());
    globalstring.append(" ");
    //emit_insn( tree->children.at(1)->lexinfo->c_str(),tree);
    emit(tree->children.at(1));
    globalstring.append(";\n");
  }
}

void emit_vardecl_old (astree* tree){
  currstring.append(eightspaces);
  if(tree->children.at(0)->children.size() == 1){
    if(tree->blocknr != 0){
      emit_insn(tree->children.at(0)->lexinfo->
                c_str(), tree, currstring);
      currstring.append(" ");
    }else{
      emit_insn(tree->children.at(0)->lexinfo->
                c_str(), tree, ocstring);
    }
    blocknr_prefix(tree);
    emit_insn(tree->children.at(0)->children.at(0)->
              lexinfo->c_str(),tree, currstring);
    /*
    emit(tree->children.at(0));
    currstring.append(" ");
    emit(tree->children.at(0)->children.at(0));
    */
  }
  currstring.append(" ");
  emit_insn( tree->lexinfo->c_str(), tree, currstring);
  currstring.append(" ");
  //emit_insn( tree->children.at(1)->lexinfo->c_str(),tree);
  emit(tree->children.at(1));
  currstring.append(";\n");
}

void blocknr_prefix(astree* tree){
  if(global){
    ocstring.append(tree->children.at(0)->lexinfo->
                    c_str());
    ocstring.append("__");
    ocstring.append(tree->children.at(0)->children.at(0)->
                    lexinfo->c_str());
    ocstring.append(";\n");
    globalstring.append("__");
  }
  else{
    globalstring.append(tree->children.at(0)->lexinfo->
                        c_str());
    globalstring.append("_%d_", tree->blocknr); 
  }
}

void postorder_emit_stmts (astree* tree) {
  //fprintf(stdout, "IN POSTORDER_EMIT_STMTS\n");
   postorder (tree);
}

void postorder_emit_binop (astree* tree) {
  //postorder (tree);
  emit(tree->children.at(0));
  if(global)
    globalstring.append(tree->lexinfo->c_str());
  else{}
    
  emit(tree->children.at(1));
}

void postorder_emit_unop (astree* tree) {
  globalstring.append( tree->lexinfo->c_str());
  emit(tree->children.at(0));
}

void emit_semi (astree* tree) {
  ///postorder (tree);
  if(global)
    globalstring.append( tree->lexinfo->c_str());
  emit_newline();
}

void emit_push (astree* tree) {
  if(global)
    globalstring.append(tree->lexinfo->c_str());
}

void emit_assign (astree* tree) {
   assert (tree->children.size() == 2);
   astree* left = tree->children.at(0);
   emit (tree->children.at(1));
   if (left->symbol != TOK_IDENT) {
      eprintf ("%:%s: %d: left operand of `=' is not an identifier\n",
               scanner_filename (left->filenr)->c_str(), left->linenr);
   }else{
      emit_insn (left->lexinfo->c_str(), left, currstring);
   }
}

void emit_if(){
  funcstring.append("");
}

void emit_while(){
  funcstring.append("");
}

void emit_return(){
  funcstring.append("");
}

void emit_new(){
  funcstring.append("");
}

void emit_temps(){
  funcstring.append("");
}


void emit (astree* tree) {
  // fprintf(stdout, "EMIT TREE SYMBOL: %s\n",
  //   get_yytname(tree->symbol));
   switch (tree->symbol) {
      case TOK_ROOT  : postorder_emit_stmts (tree);       break;
      case ';'   : emit_semi (tree);        break;
      case TOK_STRUCT : emit_struct(tree); break;
      case TOK_FUNCTION: emit_function(tree); break;
      case TOK_VARDECL: emit_vardecl (tree); break;
      case '='   : emit_assign (tree);                break;
      case '+'   : 
      case '-'   : 
      case '*'   :
      case '/'   : 
      case '%'   :
      case TOK_EQ : 
      case TOK_NE:
      case TOK_LT : 
      case TOK_LE : 
      case TOK_GT : 
      case TOK_GE   : postorder_emit_binop (tree); break;
      case '!'   :
      case TOK_ORD :
      case TOK_CHR :
      case TOK_POS   : 
      case TOK_NEG   : postorder_emit_unop (tree); break;
      case TOK_IDENT : emit_push (tree);       break;
      case TOK_TRUE :
      case TOK_FALSE :      
      case TOK_CHARCON :
      case TOK_STRINGCON :
      case TOK_INTCON : emit_push (tree);       break;
    //default    : assert (false);         break;
   }
}

void emit_sm_code (astree* tree) {
  /*
  switch_current_string(globalstring);
  point_to_string(currstring);
  */

  //fprintf (stdout, "\n");
   if (tree) {
     //fprintf(stdout,"valid tree\n");
     emit (tree);
   }
   //fprintf (stdout, currstring.c_str());
   globalstring.append("}\n");
   //printf("%s\n",globalstring.c_str());
   //printf("%s\n",ocstring.c_str());
   emit_oc_string(oilfile);
   emit_struct_string(oilfile);
   emit_func_string(oilfile);
   emit_global_string(oilfile); 
   /*
   const char* c = (*strptr).c_str();
   fprintf (stdout, c);
   fprintf (stdout, "EMIT END\n");   
   */
}

void emit_global_string(FILE* file){
  fprintf( file,"%s", globalstring.c_str());
  //fprintf( stdout, globalstring.c_str());
}

void emit_oc_string(FILE* file){
  fprintf( file,"%s", ocstring.c_str());
  //fprintf( stdout, ocstring.c_str());
}

void emit_struct_string(FILE* file){
  fprintf( file,"%s", structstring.c_str());
  //fprintf( stdout, structstring.c_str());
}

void emit_stringcon_string(FILE* file){
  fprintf( file,"%s", stringstring.c_str());
  //fprintf( stdout, stringstring.c_str());
}

void emit_func_string(FILE* file){
  fprintf( file,"%s", funcstring.c_str());
  //fprintf( stdout, funcstring.c_str());
}

void switch_current_string (string s){
  currstring = s;
}

void point_to_string (string s){
  strptr = &s;
}
RCSC("$Id: emit.cc,v 1.3 2013-09-20 17:52:13-07 - - $")

