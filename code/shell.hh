#ifndef shell_hh
#define shell_hh

#include "command.hh"

struct Shell {

  static void prompt();

  static void parseCommandLine(const char *line);
  static Command _currentCommand;
};

#endif
