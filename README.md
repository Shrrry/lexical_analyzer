# Multi-Language Lexical Analyzer and Parser

> A compiler front-end tool that accepts source code, automatically identifies the programming language, tokenizes the input, constructs a symbol table, and renders parse trees — served through a live web dashboard.

---

## Overview

This project implements the **first phase of a compiler** — lexical analysis — extended with parse tree generation and a browser-based interface. Source code entered in the dashboard is processed entirely on the backend: language detection, tokenization, symbol resolution, and tree construction all happen in compiled C parsers invoked by a Python/Flask server.

The system supports four languages — **C, C++, Java, and Python** — each handled by its own dedicated parser binary.

---

## Architecture

```
+---------------------------+
|      dashboard.html       |
|   CodeMirror Code Editor  |
|   POST /run  (JSON)       |
+------------+--------------+
             |
             v
+---------------------------+
|         app.py            |
|   Flask Web Server        |
|                           |
|  1. detect_language()     |
|     Scans code for known  |
|     patterns per language |
|                           |
|  2. Maps language to      |
|     parser executable     |
|                           |
|  3. subprocess.Popen()    |
|     Pipes code via stdin  |
+------------+--------------+
             |
     +-------+--------+
     |                |
     v                v
+---------+      +-----------+
| parser  |      | parser_   |
| .exe    |      | cpp.exe   |  ...and parser_java.exe
| (C)     |      | (C++)     |      parser_python.exe
+---------+      +-----------+
     |
     v
+---------------------------+
|    Parser Output          |
|  - Token list             |
|  - Symbol table           |
|  - Parse tree(s)          |
+---------------------------+
             |
             v
+---------------------------+
|   Browser Output Panel    |
|   Monospace terminal view |
+---------------------------+
```

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
lexical_analyzer/
|
|-- app.py                      Flask server: language detection, subprocess routing
|-- dashboard.html              Browser UI: CodeMirror editor + output panel
|
|-- lexical_parser.c            Parser source — C language
|-- lexical_parser_cpp.c        Parser source — C++ language
|-- lexical_parser_java.c       Parser source — Java language
|-- lexical_parser_python.c     Parser source — Python language
|
|-- parser.exe                  Compiled parser — C           (Windows)
|-- parser_cpp.exe              Compiled parser — C++         (Windows)
|-- parser_java.exe             Compiled parser — Java        (Windows)
|-- parser_python.exe           Compiled parser — Python      (Windows)
|
|-- lexical_parser.exe          Extended parser — C           (Windows)
|-- lexical_parser_cpp.exe      Extended parser — C++         (Windows)
|-- lexical_parser_java.exe     Extended parser — Java        (Windows)
|-- lexical_parser_python.exe   Extended parser — Python      (Windows)
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
