Regex Patterns
│
▼
┌──────────────┐
│  NFABuilder  │  Thompson's Construction Algorithm
└──────┬───────┘
│  NFA State Graph
▼
┌──────────────┐
│ DFAConverter │  Powerset / Subset Construction
└──────┬───────┘
│  Raw DFA
▼
┌──────────────┐
│ Minimize DFA │  Hopcroft's Minimization + Byte Equivalence Compression
└──────┬───────┘
│  Minimized Transition Tables
▼
┌──────────────┐
│ Code Gen     │  Generates standalone C++ Tokenizer class
└──────────────┘

---

## Supported Regex Syntax

| Syntax | Description | Example |
|---|---|---|
| `a`, `abc` | Literal characters and concatenations | `cat` |
| `a\|b` | Alternation (OR) | `true\|false` |
| `*`, `+`, `?` | Zero-or-more, One-or-more, Optional | `a*`, `b+`, `colou?r` |
| `{n}`, `{n,m}`, `{n,}` | Exact, bounded, or unbounded repetition | `\\d{4}`, `[a-z]{2,4}` |
| `[a-z]`, `[^0-9]` | Character classes and negated character classes | `[A-Za-z0-9_]` |
| `\\d`, `\\w`, `\\s` | Shorthand digits (`0-9`), word characters, whitespace | `\\w+` |
| `\\n`, `\\r`, `\\t`, `\\\\` | Escape sequences | `\\t+` |
| `(...)` | Grouping for operator precedence | `(ab)+` |

---

## Quick Start

### 1. Generating a Tokenizer

Include the header, register your regex rules with associated Token IDs, and generate the source code for your tokenizer:

```cpp
#include <iostream>
#include <fstream>
#include "RegexCompiler.hpp" // Or whatever name you saved the header as

int main() {
    RegexCompiler compiler;

    // Define Token IDs
    enum TokenID {
        KEYWORD = 1,
        IDENTIFIER,
        NUMBER,
        OPERATOR,
        WHITESPACE
    };

    // Add rules using addRules initializer list
    compiler.addRules({
        { "if|else|while|return", KEYWORD,    false },
        { "[a-zA-Z_][a-zA-Z0-9_]*", IDENTIFIER, false },
        { "\\\\d+(\\\\.\\\\d+)?",       NUMBER,     false },
        { "==|!=|\\\\+|\\\\-|\\\\*|/",   OPERATOR,   false },
        { "\\\\s+",                 WHITESPACE, false }
    });

    // Generate C++ code for a class named "Lexer"
    std::string code = compiler.generateCppClass("Lexer");

    // Output code to a header file
    std::ofstream out("Lexer.hpp");
    out << code;
    out.close();

    std::cout << "Lexer header generated successfully!\\n";
    return 0;
}

2. Using the Generated Tokenizer
The generated class (Lexer.hpp in the example above) is entirely self-contained. You can use it in your project as follows:
#include <iostream>
#include "Lexer.hpp"

int main() {
    std::string source = "if (x == 42) return y;";

    // Run the generated static tokenizer
    auto tokens = Lexer::tokenize(source);

    for (const auto& token : tokens) {
        std::cout << "Token ID: " << token.id 
                  << " | Lexeme: \\"" << token.lexeme << "\\""
                  << " | Pos: " << token.position << "\\n";
    }

    return 0;
}

Output Token Structure
The generated class includes a lightweight Token structure:
struct Token {
    int id;            // Matches the token_id passed during rule definition
                       // (-1 indicates an unmatched / fallback character)
    std::string lexeme;// The substring matched from the input
    size_t position;   // Starting byte position in the input string
};

Requirements
 * Compiler: C++17 compatible compiler (GCC 8+, Clang 7+, MSVC 2017+).
 * Header Dependencies: <iostream>, <vector>, <string>, <memory>, <set>, <map>, <stdexcept>, <cctype>, <algorithm>, <sstream>.
License
This source code is provided as-is. Feel free to use and modify it in your own projects.

README Summary
The README.md file includes:
 * Project Overview: Explains the library's purpose as a header-only C++17 regex compiler and lexer code generator written by Paul Baxter.
 * Key Features: Highlights Thompson's NFA construction, Powerset DFA conversion, Hopcroft's DFA minimization, byte equivalence class compression, and standalone C++ class code generation.
 * Pipeline Architecture: Visual ASCII diagram detailing the step-by-step translation process.
 * Supported Regex Syntax: Reference table for regex operators (*, +, ?, {n,m}, [...], \d, \w, \s, etc.).
 * Quick Start Guide: Complete, compile-ready code examples demonstrating how to construct a RegexCompiler instance, generate a Lexer.hpp header, and consume the generated static tokenize() interface.
 * Token Structure & Requirements: Documents the generated Token layout and necessary C++17 dependencies.
