

/*
 *
 * CS-252 
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <cstring>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>  // Add this to declare waitpid.
#include "y.tab.hh"


static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
  unput(c);
}

// Declare read_line
extern "C" char * read_line();

// Custom getc override
int mygetc(FILE * f) {
  static char *p = nullptr;
  char ch;

  if (!isatty(0)) {
    return getc(f);
  }

  if (p == nullptr || *p == 0) {
    char * s = read_line();
    if (s == nullptr) return EOF;
    p = s;
  }

  ch = *p;
  ++p;

  return ch;
}

#undef getc
#define getc(f) mygetc(f)


%}

%option noyywrap

%%

\n {
  return NEWLINE;
}

[ \t] {
  /* Discard spaces and tabs */
}

">" {
  return GREAT;
}

">>" {
  return GREATGREAT;
}

"<" {
  return LESS;
}

"2>" {
  return GREAT2;
}

">&" {
  return GREATAMPERSAND;
}

">>&" {
  return GREATGREATAMPERSAND;
}

"|" {
  return PIPE;
}

"&" {
  return AMPERSAND;
}

\$\([^\n]*\) {
	/* 2.8: Subshell*/
	//get rid of dollar and parents
	std::string raw = yytext;
	std::string innerCommand = raw.substr(2, raw.length() - 3);
  char *subshellCmd = const_cast<char*>(innerCommand.c_str());

  //save
	int dupin = dup(0);
	int dupout = dup(1);

	int pin[2];	//subshell
	pipe(pin);

	int pout[2];	//outshell
	pipe(pout);
	//subshell command in


	write(pin[1], subshellCmd, strlen(subshellCmd));
	write(pin[1], "\n", 1);
	close(pin[1]);	

	//subshell command out
	dup2(pin[0], 0);	
	close(pin[0]);	

	dup2(pout[1], 1);	
	close(pout[1]);	

	int ret = fork();

	if (ret == 0) {	
    //in child process
		execvp("/proc/self/exe", NULL);
		_exit(1);
	} else if (ret < 0) {
		perror("fork");
		exit(1);
	}

	dup2(dupin, 0);
	dup2(dupout, 1);
	close(dupin);
	close(dupout);

  std::string result;
  char ch;
	while (read(pout[0], &ch, 1) > 0) {
    if (ch == '\n') {
        result += ' ';  
    } else {
        result += ch;
    }
}

	//read backward
	for (int k = result.length() - 1; k >= 0; --k) {
    unput(result[k]);
  }
}

\"[^\n\"]*\" {
  // Rule to match double-quoted strings like "file name with space"
  std::string raw = yytext;
  std::string unquoted = raw.substr(1, raw.length() - 2); // remove quotes
  yylval.cpp_string = new std::string(unquoted);
  return WORD;
}

([^ \t\n\\><&|"]|\\.)+ {
  std::string text = yytext;
  std::string result;
  for (size_t i = 0; i < text.size(); i++) {
    if (text[i] == '\\' && i + 1 < text.size()) {
      // Skip the backslash and take the next character literally.
      result.push_back(text[++i]);
    } else {
      result.push_back(text[i]);
    }
  }
  yylval.cpp_string = new std::string(result);
  return WORD;
}

[^ \t\n|><][^ \t\n|><]*  {
  /* Assume that file names have only alpha chars */
  yylval.cpp_string = new std::string(yytext);
  return WORD;
}

