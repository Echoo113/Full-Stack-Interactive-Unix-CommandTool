
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

char* generateRegexForWildcard(char *dir);


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



int capacity;
int count;
char **entries;

char *buildNewPrefix(const char *base, const char *name) {
    char *result = (char *)malloc(150);
    if (!strcmp(base, ".")) {
        result = strdup(name);
    } else if (!strcmp(base, "/")) {
        sprintf(result, "%s%s", base, name);
    } else {
        sprintf(result, "%s/%s", base, name);
    }
    return result;
}



char* generateRegexForWildcard(char *dir) {
    char *reg = (char *)malloc(2 * strlen(dir) + 10);
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
    return reg;
}

void addEntry(const char *fullPath, const char *fallbackName) {
    if (count == capacity) {
        capacity *= 2;
        entries = (char **)realloc(entries, capacity * sizeof(char *));
    }

    if (fullPath && fullPath[0] != '\0') {
        entries[count++] = strdup(fullPath);
    } else {
        entries[count++] = strdup(fallbackName);
    }
}


void expandWildCards(char *prefix, char *arg) {
    char *argCursor = arg;

    if (argCursor == nullptr || *argCursor == '\0') {
    if (prefix) {
        Command::_currentSimpleCommand->insertArgument(new std::string(prefix));
    }
    return;
    }

    char *originalBuffer = (char *)malloc(strlen(arg) + 10);  
    char *bufferCursor = originalBuffer;
    char *currentComponent = bufferCursor;


    // Extract the current path component before '/'
    while (*argCursor && (bufferCursor == originalBuffer || *argCursor != '/')) {
    *bufferCursor = *argCursor;
    bufferCursor++;
    argCursor++;
    }
    *bufferCursor = '\0';


    if (strchr(currentComponent, '*') || strchr(currentComponent, '?')) {
        if (!prefix && arg[0] == '/') {
            prefix = strdup("/");
            currentComponent++;
        }

        char *regexPattern = generateRegexForWildcard(currentComponent);
        char *dirToOpen = strdup((prefix) ? prefix : ".");

        regex_t regexCompiled;
        int regexStatus = regcomp(&regexCompiled, regexPattern, REG_EXTENDED | REG_NOSUB);
        DIR *directory = opendir(dirToOpen);
        if (directory == NULL) {
            free(dirToOpen);
            free(regexPattern);
            free(bufferCursor);
            return;
        }

        struct dirent *entry;
        regmatch_t match;
        while ((entry = readdir(directory)) != NULL) {
            if (!regexec(&regexCompiled, entry->d_name, 1, &match, 0)) {
              // If more path components exist, recurse into directories
                if (*argCursor) {
                    if (entry->d_type == DT_DIR) {

                        char *newPrefix = buildNewPrefix(dirToOpen, entry->d_name);

                        expandWildCards(newPrefix, (*argCursor == '/') ? ++argCursor : argCursor);
                        free(newPrefix);
                    }
                } else {
                  
                    
                    char *fullPath = (char *)malloc(100);
                    fullPath[0] = '\0';

                    if (prefix) {
                        sprintf(fullPath, "%s/%s", prefix, entry->d_name);
                    }
                    // Skip hidden files unless user explicitly includes '.'
                    if (currentComponent[0] != '.' && entry->d_name[0] == '.') {
                        free(fullPath);
                        continue;
                    }

                   addEntry(fullPath, entry->d_name);

                    free(fullPath);
                }
            }
        }

        closedir(directory);
        regfree(&regexCompiled);
        free(dirToOpen);
        free(regexPattern);
    } else {
        char *newPrefix = (char *)malloc(100);
        if (prefix) {
            sprintf(newPrefix, "%s/%s", prefix, currentComponent);
        } else {
            newPrefix = strdup(currentComponent);
        }

        if (*argCursor){
          expandWildCards(newPrefix, ++argCursor);
          }
        free(newPrefix);
    }

    free(originalBuffer);
}



void expandWildCardsIfNecessary(char *arg) {
    capacity = 20;
    count = 0;
    entries = (char **)malloc(capacity * sizeof(char *));

    // Check if arg contains wildcards
    if ((strchr(arg, '*') || strchr(arg, '?')) &&
        !(strncmp(arg, "$", 1) == 0 || strstr(arg, "${") != nullptr)) {
        
        expandWildCards(NULL, arg);
        auto cmp = [](const void *file1, const void *file2) {
            const char *f1 = *(const char **)file1;
            const char *f2 = *(const char **)file2;
            return strcmp(f1, f2);
        };
        qsort(entries, count, sizeof(char *), cmp);

        if (count > 0) {
            for (int i = 0; i < count; i++) {
                Command::_currentSimpleCommand->insertArgument(new std::string(entries[i]));
            }
        }else {

    Command::_currentSimpleCommand->insertArgument(new std::string(arg));
}

    } else {

        Command::_currentSimpleCommand->insertArgument(new std::string(arg));
    }
    for (int i = 0; i < count; i++) {
        free(entries[i]);   
    }
    free(entries); 
}


void yyerror(const char * s) {
  fprintf(stderr, "%s", s);
} 
