# Full-Stack Interactive Unix Command Tool

Welcome to my handcrafted **Unix shell**, a deep dive into **systems programming** that simulates how real-world shells like `bash` and `csh` operate under the hood.

---

## What’s a Shell?

A **shell** is a command-line interface between users and the operating system. It interprets user commands and delegates tasks to the system, such as executing programs, managing files, and handling processes.

This project reimagines that core functionality — building shell features from scratch and enhancing them with **interactive capabilities**, including command history, environment variable expansion, wildcard matching, and autocompletion.

---

## Features Implemented

### 1. Command Parsing & Execution
- Built a custom shell grammar using **Flex and Bison (Lex & Yacc)**.
- Executed commands via `fork()` and `execvp()`.
- Supported input/output redirection (`>`, `<`, `2>`, `>>`, `>>&`) using `dup2()`.
- Implemented piping (`|`) for inter-process communication.

**Skills Applied:** IPC, process control, parsing, file descriptor manipulation.

---

### 2. Signal Handling
- Implemented graceful interruption via `SIGINT` (Ctrl+C).
- Cleaned up zombie processes using `SIGCHLD` and `waitpid()`.

**Skills Applied:** Signal handling, asynchronous processing.

---

### 3. Built-in Commands
- Implemented `cd`, `exit`, `printenv`, `setenv`, and `unsetenv`.
- Enabled environment manipulation and directory navigation.

**Skills Applied:** String handling, shell state management, memory safety.

---

### 4. Environment Variable Expansion
- Supports `${VAR}`, `${$}`, `${?}`, `${!}`, `${_}`.
- Dynamically substitutes variables during runtime.

**Skills Applied:** String parsing, runtime state tracking.

---

### 5. Wildcard Expansion
- Supports `*` and `?` for filename pattern matching.
- Uses regex and filesystem traversal (`opendir()`/`readdir()`).

**Skills Applied:** Regular expressions, file system programming.

---

### 6. Line Editing
- Wrote a custom line editor in raw terminal mode.
- Supports cursor movement, backspace/delete, and home/end keys.
- Enabled command history navigation with arrow keys.

**Skills Applied:** Terminal control, user interaction design.

---

### 7. Path Completion
- Implemented autocompletion using the Tab key.
- Dynamically completes file and directory paths.

**Skills Applied:** Real-time input parsing, filesystem introspection.

---

## Features In Progress

- **Process Substitution** (e.g., `<(cmd)`): Work in progress. Will use named pipes and advanced redirection handling.

---

## Extra Touches

- **Custom Prompt:** Supports configuration via `$PROMPT`.
- **Error Feedback:** Displays messages using `ON_ERROR` for failed commands.

---

## Manual Testing

- Verified all line editing functionality and cursor navigation.
- Tested wildcard edge cases and autocompletion under nested directories.

---

## Skills Demonstrated

| Domain                 | Skills Applied                                                         |
|------------------------|------------------------------------------------------------------------|
| Systems Programming    | Fork/exec, signals, redirection, zombie cleanup                        |
| Language Parsing       | Command grammar via Lex/Yacc                                           |
| Memory Management      | Manual `malloc`/`free`, Valgrind for leak checking                     |
| CLI/UX Design          | Custom line editor, autocompletion, command history                    |
| Filesystem Handling    | Directory traversal, regex-based matching                              |

---

## Tech Stack

- **C** — Core implementation
- **Flex & Bison** — Parsing logic
- **Valgrind** — Memory safety validation
- **Makefile** — Build automation

---

## Final Thoughts

This shell project offered an immersive experience in systems-level thinking. From managing processes to creating a smooth command-line interface, each feature helped deepen my understanding of how operating systems work behind the scenes.

> “Why use someone else’s shell when you can build your own?”

---

Feel free to explore the codebase or reach out with questions.
