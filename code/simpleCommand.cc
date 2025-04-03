#include <cstdio>
#include <cstdlib>

#include <iostream>

#include "simpleCommand.hh"


#include <regex>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>    // for getpid() and readlink()
#include <limits.h>    // for PATH_MAX
#include <sys/stat.h> // for stat()
#include <dirent.h>
#include <regex.h>
#include<pwd.h>

SimpleCommand::SimpleCommand() {
  _arguments = std::vector<std::string *>();
}

SimpleCommand::~SimpleCommand() {
  // iterate over all the arguments and delete them
  for (auto & arg : _arguments) {
    delete arg;
  }
}



/*
std::string expandEnvironment(const std::string &input) {
    std::string result = input;
    std::regex varRegex("\\$\\{([^}]+)\\}");
    std::smatch match;
    // Loop until no more ${...} patterns are found.
    while (std::regex_search(result, match, varRegex)) {
        std::string varName = match[1].str();
        // Look up the variable in the environment.
        const char *envValue = getenv(varName.c_str());
        std::string varValue = (envValue != nullptr) ? envValue : "";
        // Replace the matched pattern with its expansion.
        result.replace(match.position(0), match.length(0), varValue);
    }
    return result;
}*/

extern int lastReturnCode;
extern int lastBackgroundPid;
extern std::string lastArgument;

// Helper function to retrieve the shell's executable path.
std::string getShellPath() {
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len != -1) {
        buf[len] = '\0';
        return std::string(buf);
    }
    return "";
}
// Retrieve the value for our special variables.
std::string getSpecialVar(const std::string &varName) {
  if (varName == "$") {
      return std::to_string(getpid());
  } else if (varName == "?") {
      return std::to_string(lastReturnCode);
  } else if (varName == "!") {
      return std::to_string(lastBackgroundPid);
  } else if (varName == "_") {
      return lastArgument;
  } else if (varName == "SHELL") {
      return getShellPath();
  }
  return "";
}


// Expand all occurrences of ${VAR} in the input string.
std::string expandEnvironment(const std::string &input) {
  std::string result = input;
  std::regex varRegex("\\$\\{([^}]+)\\}");
  std::smatch match;
  while (std::regex_search(result, match, varRegex)) {
      std::string varName = match[1].str();
      std::string varValue;
      // Check if it's one of our special variables.
      if (varName == "$" || varName == "?" || varName == "!" ||
          varName == "_" || varName == "SHELL") {
          varValue = getSpecialVar(varName);
      } else {
          const char *envValue = getenv(varName.c_str());
          varValue = (envValue != nullptr) ? envValue : "";
      }
      // Replace the matched pattern with the variable's value.
      result.replace(match.position(0), match.length(0), varValue);
      if(varValue==match[0].str()){
        fprintf(stderr, "Warning: Variable %s not found\n", varName.c_str());
        break;
      }
  }
  return result;
}
  


std::string tildeExpand(const std::string &arg) {
    if (arg.empty() || arg[0] != '~') {
        return arg;  // Not a tilde expression
    }

    size_t slashPos = arg.find('/');
    std::string userPart = arg.substr(1, slashPos - 1);
    std::string rest = (slashPos == std::string::npos) ? "" : arg.substr(slashPos);

    std::string homeDir;

    if (userPart.empty()) {
        // Case: "~" or "~/..." → current user's home
        const char *home = getenv("HOME");
        if (home) {
            homeDir = home;
        } else {
            struct passwd *pw = getpwuid(getuid());
            if (pw) homeDir = pw->pw_dir;
        }
    } else {
        // Case: "~username" or "~username/..."
        struct passwd *pw = getpwnam(userPart.c_str());
        if (pw) {
            homeDir = pw->pw_dir;
        } else {
            fprintf(stderr, "Warning: No such user '%s'\n", userPart.c_str());
            return arg;  // Leave unchanged
        }
    }

    return homeDir + rest;
}

  
  





/*
char ** SimpleCommand::toArgv() {
    std::vector<std::string> expandedArgs;
    
    // Process each argument: expand environment variables.
    for (auto argPtr : _arguments) {
        std::string arg = *argPtr;
        std::string expanded = expandEnvironment(arg);
        expandWildcardsIfNecessary(expanded);
        expandedArgs.push_back(expanded);
    }
    
    // Allocate a char** array (null-terminated).
    char **argv = (char **)malloc((expandedArgs.size() + 1) * sizeof(char *));
    for (size_t i = 0; i < expandedArgs.size(); i++) {
        argv[i] = strdup(expandedArgs[i].c_str());
    }
    argv[expandedArgs.size()] = NULL;
    return argv;
}*/

char ** SimpleCommand::toArgv() {
  std::vector<std::string> finalArgs;

  for (auto argPtr : _arguments) {
      std::string arg = *argPtr;
      std::string tildeExpanded = tildeExpand(arg);
      std::string envExpanded = expandEnvironment(tildeExpanded);

      // No wildcard expansion — just keep the result
      finalArgs.push_back(envExpanded);
  }

  // Allocate char**
  char **argv = (char **)malloc((finalArgs.size() + 1) * sizeof(char *));
  for (size_t i = 0; i < finalArgs.size(); i++) {
      argv[i] = strdup(finalArgs[i].c_str());
  }
  argv[finalArgs.size()] = NULL;
  return argv;
}






void SimpleCommand::insertArgument( std::string * argument ) {
  // simply add the argument to the vector
  _arguments.push_back(argument);
}

// Print out the simple command
void SimpleCommand::print() {
  for (auto & arg : _arguments) {
    std::cout << "\"" << arg << "\" \t";
  }
  // effectively the same as printf("\n\n");
  std::cout << std::endl;
}



