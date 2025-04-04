# Full-Stack-Interactive-Unix-CommandTool
## Shell Implementation 

### Introduction
This project focuses on building a custom shell interpreter, incorporating behaviors from common shells like bash and csh. The shell is implemented in stages, starting with the parsing of commands, followed by their execution, handling signals, implementing subshells, expanding environment variables, and adding advanced features like line editing and wildcard expansion. Through this project, I demonstrated my understanding of systems programming, process management, and memory handling.

### Features Implemented

1. **Command Parsing & Execution**
   - **Complex Command Parsing**: Utilized **Lex** and **Yacc (Flex and Bison)** to implement a robust shell grammar, enabling the parsing of complex commands involving multiple arguments, pipes, and redirection.
   - **Process Execution**: Used `fork()` and `execvp()` to execute commands in child processes, ensuring correct execution flow and the ability to handle multiple processes.
   - **Redirection Handling**: Implemented file redirection for input and output (`>`, `<`, `2>`, `>>`), and combined redirection (`>&`, `>>&`), with proper file descriptor management using `dup2()`.
   - **Pipes Implementation**: Integrated pipes (`|`) to connect multiple commands, redirecting the output of one command to the input of the next, effectively implementing process communication.

   **Skills Showcased**: Systems programming, process management, inter-process communication (IPC), redirection handling, Lex and Yacc (Flex and Bison) parsing.

2. **Signal Handling**
   - **Ctrl-C Support**: Implemented `SIGINT` signal handling to stop running commands, improving the user experience by making it behave like common shells.
   - **Zombie Process Handling**: Added a signal handler for `SIGCHLD` to clean up background processes (zombie processes) using `waitpid()`, preventing unnecessary resource consumption.

   **Skills Showcased**: Signal handling, process control, memory management, process synchronization.

3. **Built-in Functions**
   - **Environment Variable Management**: Created built-in commands (`printenv`, `setenv`, `unsetenv`, and `cd`) to manage environment variables and navigate directories.
   - **Exit Command**: Implemented a graceful shutdown for the shell with a custom exit message, handling process cleanup and memory management.

   **Skills Showcased**: Built-in function implementation, memory management, shell design.

4. **Environment Variable Expansion**
   - **Variable Expansion**: Implemented environment variable expansion, expanding variables such as `${VAR}`, `${$}`, `${?}`, and `${!}` in command arguments.
   - **Special Expansions**: Implemented special expansions such as `{$SHELL}` for the shell executable and `${_}` for the last argument in the previous command.

   **Skills Showcased**: String manipulation, environment variable handling, advanced shell behavior.

5. **Wildcard Expansion**
   - **File Matching**: Implemented wildcard expansion for file and directory names using `*` and `?`. Used **regular expressions** for matching files and **directory traversal** with `opendir()` and `readdir()`.

   **Skills Showcased**: Regular expressions, filesystem traversal, pattern matching, directory handling.

6. **Line Editing Features**
   - **Terminal Input Handling**: Developed a custom line editor to handle character-by-character input, with features like cursor movement (left, right), deletion, backspace, and home/end keys in **raw terminal mode**.
   - **Command History Navigation**: Implemented history navigation using up and down arrows to allow users to retrieve previously executed commands, enhancing usability.

   **Skills Showcased**: Terminal input handling, raw mode input, line editing, data structures (history management).

7. **Path Completion**
   - **File Path Autocompletion**: Implemented path completion using the `<tab>` key, mimicking behavior from bash and other shells for user-friendly command input.

   **Skills Showcased**: Path manipulation, autocomplete functionality, user interface design.

### Features Not Implemented
- **Process Substitution**: Due to its complexity, the implementation of process substitution using the `<(command)` syntax is still under development. This would require communication between processes using named pipes and careful management of I/O redirection.

### Extra Features Implemented
- **Customizable Prompt**: Introduced the `PROMPT` environment variable, allowing users to customize the shell prompt. If `ON_ERROR` is set, it displays the custom error message when a command fails.
- **Error Handling Prompt**: Displayed error messages based on the `ON_ERROR` environment variable for better user feedback when a command fails.

### Manual Testing Criteria
- **Line Editor Commands**: Verified all line editor functionalities, such as moving the cursor, deleting characters, and navigating command history.
- **Wildcard Expansion**: Tested wildcard expansion in various scenarios (wildcards in file/directory names).
- **Path Completion**: Checked the behavior of path autocompletion with the `<tab>` key.

---

## My Skills on Display

- **Systems Programming**: Mastery in process management, signal handling, and I/O redirection within a shell environment.
- **Advanced Parsing**: Proficient in using Lex and Yacc to build parsers for complex grammars, a crucial skill for implementing shells and other compilers.
- **Memory Management**: Careful memory handling using C, ensuring no memory leaks or resource wastage, verified with tools like Valgrind.
- **Command-Line Interface (CLI) Design**: Skilled at creating user-friendly command-line tools with interactive features like history navigation and autocomplete.
- **Efficient File System Handling**: Demonstrated efficiency in handling file I/O, wildcard expansion, and regular expressions for pattern matching and file management.

---

### Tools and Techniques Used

- **Lex and Yacc (Flex and Bison)** for parsing commands.
- **Fork and Exec** for process creation and execution.
- **Dup2** for file descriptor redirection.
- **Signal Handling** with `SIGINT` and `SIGCHLD`.
- **Regular Expressions** for wildcard expansion.
- **Raw Mode Input Handling** for terminal-based line editing.

### Final Thoughts
Through the development of this shell, I have gained valuable experience in low-level systems programming, enhancing both my coding skills and understanding of operating systems. The project allowed me to integrate complex features into a real-world application, pushing my problem-solving abilities to new heights.
