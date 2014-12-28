// $Id: main.cc,v 1.9 2014-12-01 16:34:53-08 - - $

#include <string>
using namespace std;

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

#include "stringset.h"
#include "auxlib.h"
#include "lyutils.h"
#include "astree.h" 
#include "symtab.h"
#include "emit.h"

string cpp_name = "/usr/bin/cpp";
string yyin_cpp_command;
const size_t LINESIZE = 1024;
extern FILE* tokfile;
extern FILE* astfile;
extern FILE* symfile;
extern FILE* oilfile;
FILE* tokfile;
FILE* astfile;
FILE* strfile;
FILE* symfile;
FILE* oilfile;


void chomp (char* string, char delim) {
   size_t len = strlen (string);
   if (len == 0) return;
   char* nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
}

void cpplines (FILE* pipe, char* filename) {
   int linenr = 1;
   char inputname[LINESIZE];
   strcpy (inputname, filename);
   for (;;) {
      char buffer[LINESIZE];
      char* fgets_rc = fgets (buffer, LINESIZE, pipe);
      if (fgets_rc == NULL) break;
      chomp (buffer, '\n');
      // printf ("%s:line %d: [%s]\n", filename, linenr, buffer);
      // http://gcc.gnu.org/onlinedocs/cpp/Preprocessor-Output.html

      char* savepos = NULL;
      char* bufptr = buffer;
      if(*buffer == '#'){
       continue;
      }
      for (int tokenct = 1;; ++tokenct) {
         char* token = strtok_r (bufptr, " \t\n", &savepos);
         bufptr = NULL;
         if (token == NULL) break;
         // printf ("token %d.%d: [%s]\n", linenr, tokenct, token);
         intern_stringset (token);
      }
      ++linenr;
   }
}

void scan_options (int argc, char** argv) {
   int option;
   opterr = 0;
   yy_flex_debug = 0;
   yydebug = 0;
   for(;;) {
      option = getopt (argc, argv, "D:@:ely");
      if (option == EOF) break;
      switch (option) {
         case '@': set_debugflags (optarg);   break;
         case 'D': cpp_name += " -D"+ string(optarg); 
         //printf("cpp name:%s\n", cpp_name.c_str());  
                                              break;
         case 'l': yy_flex_debug = 1;         break;
         case 'y': yydebug = 1;               break;
         default:  errprintf ("%:bad option (%c)\n", optopt); break;
      }
   }
   if (optind > argc) {
      errprintf ("Usage: %s [-ly] [filename]\n", get_execname());
      exit (get_exitstatus());
   }
   //yy_flex_debug = yy_flex_debug;
   //yydebug = yydebug; 
}

void yyin_cpp_popen (const char* filename) {
   yyin_cpp_command = cpp_name;
   yyin_cpp_command += " ";
   yyin_cpp_command += filename;
   yyin = popen (yyin_cpp_command.c_str(), "r");
   if (yyin == NULL) {
      syserrprintf (yyin_cpp_command.c_str());
      exit (get_exitstatus());
   }
}

void yyin_cpp_pclose (void) {
   int pclose_rc = pclose (yyin);
   eprint_status (yyin_cpp_command.c_str(), pclose_rc);
   if (pclose_rc != 0) set_exitstatus (EXIT_FAILURE);
}

void read_pipe(FILE* pipe, string command, char* filename){
    pipe = popen (command.c_str(), "r");
    if (pipe == NULL) {
       syserrprintf (command.c_str());
    }else {
       cpplines (pipe, filename);
       int pclose_rc = pclose (pipe);
       eprint_status (command.c_str(), pclose_rc);
    }
}

void add_ext(char* file, const char * ext, char* base){
   *file = 0; // remove file extension
   strcat(base, ext);   
} 

int main (int argc, char **argv) {
    set_execname (argv[0]);
    scan_options(argc, argv);
    char* filename = argv[argc-1];
    char* base = basename(filename);

    char* ext = strchr(base, '.');
    if (ext == NULL ){
      fprintf(stderr,"no file suffix\n");
      return EXIT_FAILURE;
    }
    if (strcmp(ext+1, "oc") != 0 ){
      fprintf(stderr, "not an oc file\n");
      return EXIT_FAILURE;
    }

    string command = cpp_name + " " + filename;
    yyin_cpp_popen(filename);

    add_ext(ext, ".sym", base);
    symfile = fopen(base, "w");
    add_ext(ext, ".ast", base);
    astfile = fopen(base, "w");
    add_ext(ext, ".tok", base);
    tokfile = fopen(base, "w");
    add_ext(ext, ".str", base);
    strfile = fopen(base, "w");
    add_ext(ext, ".oil", base);
    oilfile = fopen(base, "w");
    
    //printf("test\n");
    //printf("before yyparse\n");
    yyparse();
    //printf("after yyparse\n");

    block_stack.push_back(0);
    postorder_assign(yyparse_astree);

    //print astfile
    dump_astree(astfile, yyparse_astree);

    //print tokfile
    //yyparse takes care of yylex()

    //print strfile
    dump_stringset (strfile);

    //emit
    //emit_sm_code(yyparse_astree);

    fclose(astfile);
    fclose(tokfile);
    fclose(strfile);
    fclose(symfile);
 
    /*
    printf("GLOBAL IDENT TABLE:\n ");
    print_symbol_table(stdout, global_idents);
    printf("GLOBAL STRUCT TABLE:\n ");
    print_symbol_table(stdout, global_structs);
  
    printf("final size of the symbol_stack: %lu\n",
           symbol_stack.size());
    */
    /*
    for(unsigned i = 0 ; i < symbol_stack.size(); i++){
      if(*symbol_stack.at(i) == nullptr)
        printf("null table\n");
      print_symbol_table(stdout, *symbol_stack.at(i));
    }
    */
    
    yyin_cpp_pclose();
    yylex_destroy();

    return get_exitstatus();
}
