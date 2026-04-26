# Multi-Language Lexical Analyzer and Parser
> A compiler front-end tool that accepts source code, automatically identifies the programming language, tokenizes the input, constructs a symbol table, and renders parse trees — served through a live web dashboard.

---

## Overview

This project implements the **first phase of a compiler** — lexical analysis — extended with parse tree generation and a browser-based interface. Source code entered in the dashboard is processed entirely on the backend: language detection, tokenization, symbol resolution, and tree construction all happen in compiled C parsers invoked by a Python/Flask server.

The system supports four languages — **C, C++, Java, and Python** — each handled by its own dedicated parser binary.

---

## Architecture

```
+----------------------------------+
|        dashboard.html            |
|----------------------------------|
|  CodeMirror Code Editor          |
|  User writes code                |
|                                  |
|  Click "Run Code"                |
|                                  |
|  fetch("/run")                   |
|  Sends code as JSON to Flask     |
+----------------+-----------------+
                 |
                 v

+----------------------------------+
|             app.py               |
|----------------------------------|
| Flask Backend Server             |
|                                  |
| 1. detect_language()             |
|    Detects:                      |
|    - C                           |
|    - C++                         |
|    - Java                        |
|    - Python                      |
|                                  |
| 2. Maps language to:             |
|    a) Error Checker executable   |
|    b) Lexical Parser executable  |
|                                  |
| 3. subprocess.Popen()            |
|    Sends code to Error Checker   |
+----------------+-----------------+
                 |
                 v

+----------------------------------+
|     Language Error Checker       |
|----------------------------------|
| error_c.exe                      |
| error_cpp.exe                    |
| error_java.exe                   |
| error_python.exe                 |
|                                  |
| Performs:                        |
| - Lexical error detection        |
| - Syntax error detection         |
| - Semantic error detection       |
|                                  |
| Checks for:                      |
| - invalid identifiers            |
| - missing semicolons             |
| - undeclared variables           |
| - type mismatch                  |
| - invalid operations             |
| - missing colons (Python)        |
+----------------+-----------------+
                 |
        +--------+--------+
        |                 |
        | Errors Found?   |
        |                 |
   YES  v                 v NO

+-------------------+   +--------------------------+
| Return Errors     |   | Call Lexical Parser      |
| to Flask          |   | executable               |
+-------------------+   +------------+-------------+
                                     |
                                     v

+----------------------------------+
|     Language Lexical Parser      |
|----------------------------------|
| lexical_parser.exe   (C)         |
| parser_cpp.exe       (C++)       |
| parser_java.exe      (Java)      |
| parser_python.exe    (Python)    |
|                                  |
| Generates:                       |
| - Token List                     |
| - Symbol Table                   |
| - Parse Trees                    |
+----------------+-----------------+
                 |
                 v

+----------------------------------+
|         app.py (Flask)           |
|----------------------------------|
| Returns final output to frontend |
+----------------+-----------------+
                 |
                 v

+----------------------------------+
|      Browser Output Panel        |
|----------------------------------|
| Terminal-style output box        |
|                                  |
| Shows either:                    |
| - Compilation errors             |
| OR                               |
| - Tokens                         |
| - Symbol Table                   |
| - Parse Tree                     |
+----------------------------------+

---

## Language Detection

Language is detected in `app.py` by scanning the submitted source code for known signature patterns. No file extension is required.

| Language | Detected By                                      | Status      |
|----------|--------------------------------------------------|-------------|
| C        | `#include <stdio.h>` or `printf`                 | Supported   |
| C++      | `#include <iostream>` or `cout`                  | Supported   |
| Java     | `public class` or `System.out.println`           | Supported   |
| Python   | `def ` or `print(`                               | Supported   |
| C#       | `using system` or `Console.WriteLine`            | Detected, not supported |
| Other    | No pattern matched                               | Rejected    |

---

## What Each Parser Produces

Every parser binary (`parser.exe`, `parser_cpp.exe`, `parser_java.exe`, `parser_python.exe`) performs three stages on the input code:

### Stage 1 — Tokenization

The lexer scans the input character by character and classifies every lexeme into one of five token types:

| Token Type   | Recognized Values                                                        |
|--------------|--------------------------------------------------------------------------|
| `KEYWORD`    | `int` `float` `char` `if` `else` `while` `for` `return` `void`          |
| `IDENTIFIER` | Any alphanumeric name that is not a keyword                              |
| `NUMBER`     | Any sequence of digits                                                   |
| `OPERATOR`   | `+`  `-`  `*`  `/`  `=`                                                  |
| `SYMBOL`     | `;`  `(`  `)`  `{`  `}`                                                  |

Whitespace is consumed and discarded. Any unrecognized character is silently skipped.

### Stage 2 — Symbol Table

Every `IDENTIFIER` token is recorded in a symbol table. Duplicate entries are suppressed — each identifier appears only once regardless of how many times it is used in the code.

```
Index   Name    Type
1       a       int
2       b       int
3       c       int
```

### Stage 3 — Parse Tree

The parser scans the token stream for assignment statements of the form:

```
IDENTIFIER = <expression> ;
```

For each assignment found, it builds a binary expression tree rooted at `=`, with the left-hand side identifier on the left and the parsed expression on the right. The tree is printed to stdout with indented branches.

```
PARSE TREE 1
|-- =
    |-- a
    |-- +
        |-- b
        |-- c
```

Multiple assignments in a single input produce multiple numbered parse trees.

---

## Project Files

```
multi_language_parser/
│
├── app.py
│   Flask backend
│   - Detects language
│   - Calls error checker
│   - If no errors → calls parser
│   - Sends output to frontend
│
├── dashboard.html
│   Frontend UI
│   - CodeMirror editor
│   - Run Code button
│   - Output panel
│
│
├── ERROR CHECKERS (Source Files)
│
├── error_c.c
│   Detects C errors
│
├── error_cpp.c
│   Detects C++ errors
│
├── error_java.c
│   Detects Java errors
│
├── error_python.c
│   Detects Python errors
│
│
├── ERROR CHECKERS (Executables)
│
├── error_c.exe
├── error_cpp.exe
├── error_java.exe
├── error_python.exe
│
│
├── LEXICAL PARSERS (Source Files)
│
├── lexical_parser.c
│   C parser source
│
├── lexical_parser_cpp.c
│   C++ parser source
│
├── lexical_parser_java.c
│   Java parser source
│
├── lexical_parser_python.c
│   Python parser source
│
│
├── PARSER EXECUTABLES
│
├── lexical_parser.exe
│   C parser executable
│
├── parser_cpp.exe
│   C++ parser executable
│
├── parser_java.exe
│   Java parser executable
│
├── parser_python.exe
│   Python parser executable
│
│
└── static/ (optional future folder)
    ├── css/
    ├── js/
    └── assets/
```

---

## Setup and Usage

### Requirements

- Python 3.x
- Flask

```bash
pip install flask
```

GCC is required only if you need to recompile the parser sources.

---

### Run the Application

```bash
git clone https://github.com/Shrrry/lexical_analyzer.git
cd lexical_analyzer
python app.py
```

Open your browser at:

```
http://127.0.0.1:5000
```

Write or paste source code into the editor on the left panel and click **Run Code**. The output panel on the right displays the detected language, token list, symbol table, and parse trees.

---

### Recompile Parsers (Linux / macOS)

The repository ships with pre-compiled `.exe` binaries for Windows. On Linux or macOS, compile the parsers manually:

```bash
gcc lexical_parser.c        -o parser
gcc lexical_parser_cpp.c    -o parser_cpp
gcc lexical_parser_java.c   -o parser_java
gcc lexical_parser_python.c -o parser_python
```

The Flask server detects the OS at runtime (`os.name`) and selects the correct executable path automatically.

---

## Full Example

**Input typed into the dashboard editor:**

```c
int a;
a = b + c;
```

**Output displayed in the output panel:**

```
Language Detected: C

----- TOKENS -----
int -> KEYWORD
a -> IDENTIFIER
a -> IDENTIFIER
= -> OPERATOR
b -> IDENTIFIER
+ -> OPERATOR
c -> IDENTIFIER
; -> SYMBOL

----- SYMBOL TABLE -----
Index   Name    Type
1       a       int
2       b       int
3       c       int

PARSE TREE 1
|-- =
    |-- a
    |-- +
        |-- b
        |-- c
```

---

## Technical Stack

| Component        | Technology              | Role                                                  |
|------------------|-------------------------|-------------------------------------------------------|
| Parser Engine    | C (compiled binary)     | Tokenizer, symbol table builder, parse tree generator |
| Web Server       | Python, Flask           | Receives code, detects language, invokes parser       |
| Process Bridge   | Python subprocess       | Pipes source code to parser via stdin                 |
| Frontend Editor  | HTML, CSS, JavaScript   | Split-panel dashboard with code editor                |
| Syntax Highlight | CodeMirror 5.65.13      | Line-numbered, highlighted code input area            |

---

## Limitations

- Operators recognized: `+` `-` `*` `/` `=` only
- Parse trees are generated for assignment statements only (`identifier = expression ;`)
- Number literals are integers only (no floating point)
- C# code is detected but returns an unsupported language error
- Input with no recognizable language pattern is rejected

---

*Developed as a Compiler Design course project — demonstrating lexical analysis, symbol table construction, and parse tree generation across multiple programming languages via a unified web interface.*
