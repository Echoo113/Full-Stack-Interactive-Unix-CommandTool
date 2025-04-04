# 🚀 Full-Stack Interactive Unix Command Tool

Welcome to my handcrafted **Unix shell**! It's a deep dive into **systems programming**, designed to simulate how real-world shells like `bash` and `csh` work under the hood.

---

## 🐚 What’s a Shell, Anyway?

A **shell** is the magical translator between you and your computer. It takes what you type, interprets it, and tells your system what to do — whether that’s running a program, moving files, or managing processes. Think of it as the terminal’s brain 🧠.

This project reimagines that brain — building core shell features from the ground up, then enhancing them with **interactive behaviors** like command history, environment variable expansion, wildcards, and even autocompletion.

---

## ✨ Features Implemented

### 🧠 1. Command Parsing & Execution
- Built a **custom shell grammar** using **Flex and Bison (Lex & Yacc)**.
- Executed commands with `fork()` and `execvp()`.
- Supported **input/output redirection** (`>`, `<`, `2>`, `>>`, `>>&`) using `dup2()`.
- Enabled **piping** (`|`) between commands.

💡 *Skills:* IPC, process management, parsing logic, redirection, grammar construction.

---

### 🔔 2. Signal Handling
- **Ctrl+C** (`SIGINT`) gracefully interrupts commands.
- **Zombie cleanup** via `SIGCHLD` and `waitpid()` ensures no dangling processes.

💡 *Skills:* Signal control, async process handling, system-level interrupts.

---

### 🔧 3. Built-in Commands
- Implemented `cd`, `exit`, `printenv`, `setenv`, and `unsetenv`.
- Controlled directory navigation and environment settings.

💡 *Skills:* Shell logic, string and environment handling, memory cleanup.

---

### 💬 4. Environment Variable Expansion
- Supports `${VAR}`, `${$}`, `${?}`, `${!}`, `${_}`.
- Handled **dynamic runtime values** for smarter commands.

💡 *Skills:* String manipulation, shell scripting behavior, system introspection.

---

### 🌟 5. Wildcard Expansion
- Implemented `*` and `?` using regex and file system traversal.
- Seamlessly integrates with user input for flexible command support.

💡 *Skills:* Regex, `opendir()`/`readdir()`, file matching.

---

### 🎯 6. Line Editing
- Wrote a **custom line editor** in raw terminal mode.
- Supported cursor movement, backspace/delete, and home/end.
- Navigated command history with ⬆️ / ⬇️ keys.

💡 *Skills:* Terminal I/O, raw input control, interactive UI building.

---

### ⌨️ 7. Path Completion
- Autocompletion with `<tab>`, like in `bash`.
- Dynamically lists matching file paths.

💡 *Skills:* Real-time input handling, file path resolution.

---

## ⚠️ Features Not Yet Implemented
- **Process substitution** (e.g., `<(cmd)`) is in progress. It involves named pipes and deep I/O redirection logic — definitely on the roadmap!

---

## 💡 Extra Touches
- ✨ **Custom Prompt**: Personalize with `$PROMPT`.
- ❌ **On-Error Feedback**: Custom error messages via `ON_ERROR` when commands fail.

---

## ✅ Manual Testing Highlights
- Verified all line editing actions (cursor moves, backspace, history).
- Tested wildcard expansion under edge cases.
- Path autocompletion under varied directory structures.

---

## 🛠️ Skills Demonstrated

| Domain                    | Skills Applied                                                             |
|--------------------------|-----------------------------------------------------------------------------|
| 💻 Systems Programming    | Fork/exec, signals, redirection, zombie handling                            |
| 📜 Language Parsing       | Lex/Yacc grammar for robust command parsing                                 |
| 🧠 Memory Management      | Manual `malloc`/`free`, Valgrind-tested memory safety                       |
| 🎨 UI/CLI Design          | Usable interactive interface, autocompletion, navigation                    |
| 🗂️ Filesystem Handling    | Regex-based matching, traversal, autocomplete                               |

---

## 🔧 Tech Stack

- `C` — Core implementation
- `Flex & Bison` — Command parsing
- `Valgrind` — Memory leak checking
- `Makefile` — Build automation

---

## 🎯 Final Thoughts

Building this shell taught me more than just code — it taught me how a real system thinks. From process control to user-friendly interaction design, this project sharpened my skills across the board. It’s not just a tool; it’s a learning milestone I’m proud of.

> “Why use someone else’s shell when you can build your own?” 😄

---

Feel free to explore, fork, or reach out if you have questions!
