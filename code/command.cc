

#include <cstdio>
#include <cstdlib>

#include "command.hh"
#include "shell.hh"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <regex>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
extern int yyparse(void);
extern int lastReturnCode;
extern int lastBackgroundPid;
extern std::string lastArgument;
extern "C" void tty_restore_mode(void);

Command::Command() {
  // Initialize a new vector of Simple Commands
  _simpleCommands = std::vector<SimpleCommand *>();

  _outFile = NULL;
  _inFile = NULL;
  _errFile = NULL;
  _background = false;
  _append = 0;
  _outputRedirectCount = 0;
  _inputRedirectCount = 0;
  // printf("outputRedirectCount ini: %d\n", _outputRedirectCount);
  // printf("inputRedirectCount: %d\n", _inputRedirectCount);
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand) {
  // add the simple command to the vector
  _simpleCommands.push_back(simpleCommand);
}

void Command::clear() {
  // deallocate all the simple commands in the command vector
  for (auto simpleCommand : _simpleCommands) {
    delete simpleCommand;
  }

  // remove all references to the simple commands we've deallocated
  // (basically just sets the size to 0)
  _simpleCommands.clear();

  if (_outFile) {
    delete _outFile;
  }
  _outFile = NULL;

  if (_inFile) {
    delete _inFile;
  }
  _inFile = NULL;

  if (_errFile) {
    delete _errFile;
  }
  _errFile = NULL;

  _background = false;
  _append = 0;
  _outputRedirectCount = 0;
  _inputRedirectCount = 0;
  // printf("outputRedirectCount clear: %d\n", _outputRedirectCount);
  // printf("inputRedirectCount: %d\n", _inputRedirectCount);
}

void Command::print() {

  printf("\n\n");
  printf("              COMMAND TABLE                \n");
  printf("\n");
  printf("  #   Simple Commands\n");
  printf("  --- ----------------------------------------------------------\n");

  int i = 0;
  // iterate over the simple commands and print them nicely
  for (auto &simpleCommand : _simpleCommands) {
    printf("  %-3d ", i++);

    // Print command and arguments with memory addresses
    for (size_t j = 0; j < simpleCommand->_arguments.size(); ++j) {
      printf("%s: \"%p\" \t", simpleCommand->_arguments[j]->c_str(),
             static_cast<void *>(simpleCommand->_arguments[j]));
    }
    printf("\n");

    // simpleCommand->print();
  }

  printf("\n\n");
  printf("  Output       Input        Error        Background\n");
  printf("  ------------ ------------ ------------ ------------\n");
  printf("  %-12s %-12s %-12s %-12s\n",
         _outFile ? _outFile->c_str() : "default",
         _inFile ? _inFile->c_str() : "default",
         _errFile ? _errFile->c_str() : "default", _background ? "YES" : "NO");
  printf("\n\n");
}

bool Command::handleBuiltIn() {
  if (_simpleCommands.size() == 0)
    return false;

  // Get the command name from the first simple command.
  const char *cmd = _simpleCommands[0]->_arguments[0]->c_str();

  // Handle "cd"
  if (strcmp(cmd, "cd") == 0) {
    const char *dir;
    /*
    if (_simpleCommands[0]->_arguments.size() > 1)
        dir = _simpleCommands[0]->_arguments[1]->c_str();*/

    std::string expanded;
    if (_simpleCommands[0]->_arguments.size() > 1) {

      std::string arg = *_simpleCommands[0]->_arguments[1];

      if (arg.find("${") != std::string::npos) {
        expanded = arg;

        std::regex varRegex("\\$\\{([^}]+)\\}");
        std::smatch match;
        while (std::regex_search(expanded, match, varRegex)) {
          std::string varName = match[1].str();

          const char *envValue = getenv(varName.c_str());
          std::string varValue = (envValue != nullptr) ? envValue : "";

          expanded.replace(match.position(0), match.length(0), varValue);
        }
        dir = expanded.c_str();
      } else {
        dir = _simpleCommands[0]->_arguments[1]->c_str();
      }
    }

    else
      dir = getenv("HOME"); // default to home directory

    if (chdir(dir) < 0) {
      // perror("chdir");
      fprintf(stderr, "cd: can't cd to %s\n", dir);
    }
    return true;
  }

  // Handle "setenv"
  if (strcmp(cmd, "setenv") == 0) {
    if (_simpleCommands[0]->_arguments.size() < 3) {
      fprintf(stderr, "Usage: setenv VARIABLE VALUE\n");
    } else {
      if (setenv(_simpleCommands[0]->_arguments[1]->c_str(),
                 _simpleCommands[0]->_arguments[2]->c_str(), 1) < 0)
        perror("setenv");
    }
    return true;
  }

  // Handle "unsetenv"
  if (strcmp(cmd, "unsetenv") == 0) {
    if (_simpleCommands[0]->_arguments.size() < 2) {
      fprintf(stderr, "Usage: unsetenv VARIABLE\n");
    } else {
      if (unsetenv(_simpleCommands[0]->_arguments[1]->c_str()) < 0)
        perror("unsetenv");
    }
    return true;
  }
  
  
       

  return false;
}

void Command::execute() {
  // Don't do anything if there are no simple commands
  if (_simpleCommands.size() == 0) {
    Shell::prompt();
    return;
  }

  if (_simpleCommands.size() == 1 &&
    _simpleCommands[0]->_arguments.size() == 2 &&
    _simpleCommands[0]->_arguments[0]->compare("source") == 0) {

  const char *filename = _simpleCommands[0]->_arguments[1]->c_str();
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    perror("source");
    clear();
    Shell::prompt();
    return;
  }

  char line[1024];
  while (fgets(line, sizeof(line), fp)) {
    // Remove trailing newline
    line[strcspn(line, "\n")] = 0;

    // Split line into tokens
    std::istringstream iss(line);
    std::string token;
    SimpleCommand *cmd = new SimpleCommand();

    while (iss >> token) {
      token.erase(std::remove(token.begin(), token.end(), '\"'), token.end());
      std::string *arg = new std::string(token);
      cmd->insertArgument(arg);
    }

    // Use your own command executor!
    char **argv = cmd->toArgv();

    if (argv[0] == nullptr) {
      delete cmd;
      continue;
    }

    // Handle built-in command like setenv inside this shell
    if (strcmp(argv[0], "setenv") == 0 && argv[1] && argv[2]) {
      setenv(argv[1], argv[2], 1);
    } else if (strcmp(argv[0], "unsetenv") == 0 && argv[1]) {
      unsetenv(argv[1]);
    } else if (strcmp(argv[0], "cd") == 0) {
      const char *dir = (argv[1]) ? argv[1] : getenv("HOME");
      if (chdir(dir) < 0) {
        fprintf(stderr, "cd: can't cd to %s\n", dir);
      }
    } else {
      // External command (e.g., echo ${aaa})
      pid_t pid = fork();
      if (pid == 0) {
        execvp(argv[0], argv);
        perror("execvp");
        exit(1);
      } else {
        int status;
        waitpid(pid, &status, 0);
        lastReturnCode = WEXITSTATUS(status);
      }
    }

    // Free
    for (int i = 0; argv[i]; ++i) {
      free(argv[i]);
    }
    free(argv);
    delete cmd;
  }

  fclose(fp);
  clear();
  Shell::prompt();
  return;
}

// Print contents of Command data structure
#ifdef PRINTING
  print();
#endif
  if (strcmp(_simpleCommands[0]->_arguments[0]->c_str(), "exit") == 0) {
    printf("Good bye!!\n");
    tty_restore_mode();
    exit(1);
  }

  //  handle built-in commands executed in the parent
  if (handleBuiltIn()) {
    clear();
    Shell::prompt();
    return;
  }
  // Add execution here
  // For every simple command fork a new process
  // Setup i/o redirection
  // and call exec

  // check for ambiguous output redirect

  if (_outputRedirectCount > 1 || _inputRedirectCount > 1) {
    printf("Ambiguous output redirect.\n");
    clear();
    Shell::prompt();
    return;
    // exit(1);
  }




  // Save input, output, and error
  int tmpin = dup(0);
  int tmpout = dup(1);
  int tmperr = dup(2);

  // ---------------------------------------------------
  //  Input Redirection
  // ---------------------------------------------------

  int fdin, fdout, fderr;
  // set up initial input
  if (_inFile) {
    fdin = open(_inFile->c_str(), O_RDONLY);

  } else {
    // use default input
    fdin = dup(tmpin);
  }

  // ---------------------------------------------------
  // Loop through each simple command in the pipeline.
  // ---------------------------------------------------
  pid_t pid = 0;
  size_t numSimpleCommands = _simpleCommands.size();
  for (size_t i = 0; i < numSimpleCommands; i++) {
    // redirect input
    dup2(fdin, 0);
    close(fdin);
    // ---------------------------------------------------
    //  Pipe Setup and Output Redirection
    // ---------------------------------------------------
    // Set up output file descriptor using the specified file or the default.

    // last simple command (it determines the final output destination )
    if (i == numSimpleCommands - 1) {
      if (_outFile) {
        // output redirect
        if (_append) {
          fdout = open(_outFile->c_str(), O_WRONLY | O_APPEND | O_CREAT, 0666);
        } else {
          fdout = open(_outFile->c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0666);
        }
      } else {
        // use default output
        fdout = dup(tmpout);
      }
      // handle the error file
      if (_errFile) {
        if (_append) {
          fderr = open(_errFile->c_str(), O_WRONLY | O_APPEND | O_CREAT, 0666);
        } else {
          fderr = open(_errFile->c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0666);
        }
      } else {
        fderr = dup(tmperr);
      }
    } else {
      // create pipe for the output of the current simple command
      int fdpipe[2];
      pipe(fdpipe);
      fdout = fdpipe[1];
      fdin = fdpipe[0];
    }
    // redirect output
    dup2(fdout, 1);
    close(fdout);

    // For the last command, also redirect STDERR to fderr.
    if (i == numSimpleCommands - 1) {
      dup2(fderr, 2);
      close(fderr);
    }

    //----------------------------------------------
    // Process Creation and Execution
    //----------------------------------------------

    // Expand environment variables (ignoring wildcards) for this SimpleCommand.
    SimpleCommand *s = _simpleCommands[i];
    char **args = s->toArgv();
    pid = fork();

    if (pid == 0) {
      // child---------------------------------------------------

      // handle printenv
      if (strcmp(_simpleCommands[i]->_arguments[0]->c_str(), "printenv") == 0) {
        extern char **environ;
        for (char **p = environ; *p != NULL; p++) {
          printf("%s\n", *p);
        }
        exit(0);
      }
      



      

      execvp(args[0], (char *const *)args);
      // exit if execvp fails
      perror("execvp");
      exit(1);
      
    

    } else if (pid < 0) {
      perror("fork");
      return;
    } else {
      if (_background) {
        // Store the background process ID.
        lastBackgroundPid = pid;
      }
    }
    for (int j = 0; args[j]; ++j) {
        free(args[j]);
    }
    free(args);
  }

  // Restore in/out defaults
  dup2(tmpin, 0);
  dup2(tmpout, 1);
  dup2(tmperr, 2);
  close(tmpin);
  close(tmpout);
  close(tmperr);

  // parent
  if (_background == false) {
    int status;
    waitpid(pid, &status, 0);
    // Update lastReturnCode with the command's exit status.
    lastReturnCode = WEXITSTATUS(status);
  }
  // Update lastArgument from the parsed command.
  if (!_simpleCommands.empty()) {
    SimpleCommand *lastCmd = _simpleCommands.back();
    bool skipNext = false;

    for (auto it = lastCmd->_arguments.rbegin(); it != lastCmd->_arguments.rend(); ++it) {
        if (skipNext) {
            skipNext = false;
            continue;
        }

        std::string arg = **it;
        if (arg == ">" || arg == ">>" || arg == "<" || arg == "2>" || arg == "2>>") {
            skipNext = true; // skip the next argument (the filename)
            continue;
        }

        // Found a valid last argument
        lastArgument = arg;
        break;
    }
}

  // Clear to prepare for next command
  clear();
  
  //----------------------------------------------
  // isatty
  //----------------------------------------------
  fflush(stdout);
  // Print new prompt
  if (isatty(0)) {
    Shell::prompt();
  }

  
}

SimpleCommand *Command::_currentSimpleCommand;

