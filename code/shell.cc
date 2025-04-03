/*#include <cstdio>

#include "shell.hh"
extern int yydebug;
int yyparse(void);

void Shell::prompt() {
//  printf("myshell>");
  fflush(stdout);
}

int main() {

Shell::prompt();
//  yydebug = 1;
  yyparse();
}

Command Shell::_currentCommand;
*/




#include <cstdio>
#include <csignal>
#include <cstring>
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>
#include "shell.hh"



#include <string>

Command Shell::_currentCommand;


int lastReturnCode = 0;
int lastBackgroundPid = 0;
std::string lastArgument ;


//extern int yydebug;
int yyparse(void);

void Shell::prompt() {
  // printf("myshell>");

    fflush(stdout);

  
}

// Signal handler for SIGINT (Ctrl-C)
void sigIntHandler(int sig) {
  // When Ctrl-C is pressed, simply reprint the prompt.
  // This prevents the shell from exiting unexpectedly.
  Shell::prompt();
}

// SIGCHLD handler for zombie elimination
void sigChldHandler(int sig) {

  pid_t pid;
  // Reap all terminated children.
  while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
   
  }
}

int main() {
  struct sigaction signalAction;
  signalAction.sa_handler = sigIntHandler;
  sigemptyset(&signalAction.sa_mask);
  signalAction.sa_flags = SA_RESTART;
  int error = sigaction(SIGINT, &signalAction, NULL);
  if (error) {
    perror("sigaction");
    exit(-1);
  }
  
  // Set up SIGCHLD handler for zombie elimination.
  struct sigaction sigchldAction;
  sigchldAction.sa_handler = sigChldHandler;
  sigemptyset(&sigchldAction.sa_mask);
  sigchldAction.sa_flags = SA_RESTART;
  error = sigaction(SIGCHLD, &sigchldAction, NULL);
  if (error) {
    perror("sigaction");
    exit(-1);
  }
  
  
  Shell::prompt();
 // yydebug = 1;
  yyparse();
  
  // If yyparse returns, assume the "exit" command was entered.
  // Print the goodbye message and exit without creating a new process.
}







