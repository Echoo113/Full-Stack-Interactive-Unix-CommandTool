/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *   cmd [arg]* [ | cmd [arg]* ]* [ [> filename] [>> filename] [< filename]
 *   [2> filename] [>& filename] [>>& filename] ]* [&]
 *
 * The parsed command is stored in the Shell::_currentCommand (of type Command)
 * which uses the Command data structure defined in command.hh.
 */

%code requires {
  #include <string>
  #include <cstring>     // for strcmp, strlen, strchr, strdup
#include <cstdlib>     // for malloc, realloc, free
#include <cstdio>      // for printf, sprintf
#include <dirent.h>    // for DIR, readdir, closedir
#include <regex.h>     // for regex_t, regcomp, regexec
#include <sys/types.h> // for DT_DIR

  #if __cplusplus > 199711L
  #define register      // Deprecated in C++11 so remove the keyword
  #endif
}

%union {
  char         *string_val;
  std::string  *cpp_string;
}

/* Token definitions. */
%token <cpp_string> WORD
%token NOTOKEN GREAT NEWLINE LESS GREATGREAT GREAT2 GREATAMPERSAND GREATGREATAMPERSAND PIPE AMPERSAND

%{
#include <cstdio>
#include "shell.hh"       // Provides Shell::_currentCommand
#include "command.hh"     // Contains Command structure and definition of _currentSimpleCommand

void yyerror(const char * s);
int yylex();
void expandWildCardsIfNecessary(char * arg);
void expandWildCards(char * prefix, char * arg);
int cmpfunc(const void * file1, const void * file2);


%}

%%

goal:
    command_list
;

command_list:
      command_line
    | command_list command_line
;

command_line:
      pipe_list io_modifier_list background_optional NEWLINE {
         Shell::_currentCommand.execute();
         Shell::_currentCommand.clear();  
      }
    | NEWLINE
    | error NEWLINE { yyerrok; }
;

pipe_list:
      cmd_and_args
    | pipe_list PIPE cmd_and_args
;

cmd_and_args:
      WORD {
         Command::_currentSimpleCommand = new SimpleCommand();
         Command::_currentSimpleCommand->insertArgument($1);
      } arg_list {
         Shell::_currentCommand.insertSimpleCommand(Command::_currentSimpleCommand);
      }
;

arg_list:
      /* empty */
    | arg_list WORD {

        expandWildCardsIfNecessary(const_cast<char *>($2->c_str()));
        delete $2;
      }
;

io_modifier_list:
      io_modifier_list io_modifier
    | /* empty */
;

io_modifier:
      GREATGREAT WORD {
        #ifdef PRINTING
          printf("   Yacc: insert append output \"%s\"\n", $2->c_str());
        #endif
         Shell::_currentCommand._outFile = $2;
         Shell::_currentCommand._append = 1;
         Shell::_currentCommand._outputRedirectCount++;
      }
    | GREAT WORD {
      #ifdef PRINTING
          printf("   Yacc: insert output \"%s\"\n", $2->c_str());
        #endif
         Shell::_currentCommand._outFile = $2;
         Shell::_currentCommand._append = 0;
         Shell::_currentCommand._outputRedirectCount++;
      }
    | GREATGREATAMPERSAND WORD {
      #ifdef PRINTING
          printf("   Yacc: insert append output/error \"%s\"\n", $2->c_str());
        #endif
         Shell::_currentCommand._outFile = $2;
         Shell::_currentCommand._errFile = $2;
         Shell::_currentCommand._append = 1;
         Shell::_currentCommand._outputRedirectCount++;
      }
    /* Error redirection */
    | GREAT2 WORD {
      #ifdef PRINTING
          printf("   Yacc: insert error output \"%s\"\n", $2->c_str());
        #endif

         Shell::_currentCommand._errFile = $2;
    }
    
    | GREATAMPERSAND WORD {
      #ifdef PRINTING
          printf("   Yacc: insert output/error \"%s\"\n", $2->c_str());
        #endif
         Shell::_currentCommand._outFile = $2;
         Shell::_currentCommand._errFile = $2;
         Shell::_currentCommand._append = 0;
         Shell::_currentCommand._outputRedirectCount++;
      }
    | LESS WORD {
      #ifdef PRINTING
          printf("   Yacc: insert input \"%s\"\n", $2->c_str());
        #endif
         Shell::_currentCommand._inFile = $2;
         Shell::_currentCommand._inputRedirectCount++;

      }
;



background_optional:
      AMPERSAND {
         Shell::_currentCommand._background = true;
      }
    | /* empty */
;


%%



int maxEntries = 20;
int nEntries = 0;
char **entries;

int cmpfunc(const void *file1, const void *file2) {
    const char *_file1 = *(const char **)file1;
    const char *_file2 = *(const char **)file2;
    return strcmp(_file1, _file2);
}

void expandWildCards(char *prefix, char *arg) {
    char *temp = arg;
    char *saveOriginal = (char *)malloc(strlen(arg) + 10);  // line 179
char *save = saveOriginal;
char *dir = save;

    if (temp[0] == '/') *(save++) = *(temp++);
    while (*temp != '/' && *temp) *(save++) = *(temp++);
    *save = '\0';

    if (strchr(dir, '*') || strchr(dir, '?')) {
        if (!prefix && arg[0] == '/') {
            prefix = strdup("/");
            dir++;
        }

        char *reg = (char *)malloc(2 * strlen(arg) + 10);
        char *a = dir;
        char *r = reg;
        *(r++) = '^';
        while (*a) {
            if (*a == '*') { *(r++) = '.'; *(r++) = '*'; }
            else if (*a == '?') { *(r++) = '.'; }
            else if (*a == '.') { *(r++) = '\\'; *(r++) = '.'; }
            else { *(r++) = *a; }
            a++;
        }
        *(r++) = '$'; *r = '\0';

        regex_t re;
        int expbuf = regcomp(&re, reg, REG_EXTENDED | REG_NOSUB);
        char *toOpen = strdup((prefix) ? prefix : ".");
        DIR *d = opendir(toOpen);
        if (d == NULL) {
            free(toOpen);
            free(reg);
            free(save);  // ✅ clean up before return
            return;
        }

        struct dirent *ent;
        regmatch_t match;
        while ((ent = readdir(d)) != NULL) {
            if (!regexec(&re, ent->d_name, 1, &match, 0)) {
                if (*temp) {
                    if (ent->d_type == DT_DIR) {
                        char *nPrefix = (char *)malloc(150);
                        if (!strcmp(toOpen, ".")) nPrefix = strdup(ent->d_name);
                        else if (!strcmp(toOpen, "/")) sprintf(nPrefix, "%s%s", toOpen, ent->d_name);
                        else sprintf(nPrefix, "%s/%s", toOpen, ent->d_name);
                        expandWildCards(nPrefix, (*temp == '/') ? ++temp : temp);
                        free(nPrefix);
                    }
                } else {
                    if (nEntries == maxEntries) {
                        maxEntries *= 2;
                        entries = (char **)realloc(entries, maxEntries * sizeof(char *));
                    }
                    char *argument = (char *)malloc(100);
                    argument[0] = '\0';
                    if (prefix) sprintf(argument, "%s/%s", prefix, ent->d_name);
                    // If pattern starts with '.', include everything (including . and ..)
if (dir[0] == '.') {
    entries[nEntries++] = (argument[0] != '\0') ? strdup(argument) : strdup(ent->d_name);
}
// If not, skip hidden files (including . and ..)
else {
    if (ent->d_name[0] == '.') {
      free(argument);
        continue;
    }
    entries[nEntries++] = (argument[0] != '\0') ? strdup(argument) : strdup(ent->d_name);
}
free(argument);  





                }
            }
        }
        closedir(d);
        regfree(&re);
        free(toOpen); 
        free(reg);  
    } else {
        char *preToSend = (char *)malloc(100);
        if (prefix) sprintf(preToSend, "%s/%s", prefix, dir);
        else preToSend = strdup(dir);
        if (*temp) expandWildCards(preToSend, ++temp);
        free(preToSend);
    }
    free(saveOriginal); 
    
}

void expandWildCardsIfNecessary(char *arg) {
    maxEntries = 20;
    nEntries = 0;
    entries = (char **)malloc(maxEntries * sizeof(char *));

    // Check if arg contains wildcards
    if ((strchr(arg, '*') || strchr(arg, '?')) &&
        !(strncmp(arg, "$", 1) == 0 || strstr(arg, "${") != nullptr)) {
        
        expandWildCards(NULL, arg);
        qsort(entries, nEntries, sizeof(char *), cmpfunc);

        if (nEntries > 0) {
            for (int i = 0; i < nEntries; i++) {
                Command::_currentSimpleCommand->insertArgument(new std::string(entries[i]));
            }
        }else {
    // No match: insert original arg literally (Bash fallback behavior)
    Command::_currentSimpleCommand->insertArgument(new std::string(arg));
}

    } else {
        // No wildcard — insert literally
        Command::_currentSimpleCommand->insertArgument(new std::string(arg));
    }
    for (int i = 0; i < nEntries; i++) {
        free(entries[i]);     // ✅ ADD THIS LOOP TO FREE EACH ENTRY
    }
    free(entries); 
}


void yyerror(const char * s) {
  fprintf(stderr, "%s", s);
}           
