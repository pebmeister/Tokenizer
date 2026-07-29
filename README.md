Here is a complete, well-structured README.md tailored for your source code.
RegexCompiler
A lightweight, header-only C++17 library that compiles regular expressions into optimized Deterministic Finite Automata (DFA) and generates standalone, dependency-free C++ tokenizer classes.
Written by Paul Baxter.
Overview
RegexCompiler implements a classic compiler frontend pipeline to translate high-level regex patterns into high-performance lexers. Instead of interpreting regexes at runtime, it constructs a minimized DFA and generates a self-contained C++ class featuring O(N) tokenization speed via fast array lookups.
Key Features
 * Header-Only & Dependency-Free: Requires only standard C++17. No external libraries needed.
 * Full Lexing Pipeline:
   * NFA Engine: Built with Thompson's Construction.
   * DFA Conversion: Powerset Construction (Subset Construction).
   * Minimization: Hopcroft's Algorithm for minimal state count.
   * Table Compression: Automatically maps byte sets (0–255) to character equivalence classes to keep memory footprint tiny.
 * Maximal Munch Matching: Tokenizer defaults to longest-match rules with automatic fallback handling for unmatched characters (token_id = -1).
 * Standalone Code Generation: Emits a clean, header-only C++ class (default name: Tokenizer) containing lookup tables and a static tokenize() method.
 * Case-Insensitive Support: Per-rule flag to easily match case-insensitively.
Pipeline Architecture
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

Supported Regex Syntax
| Syntax | Description | Example |
|---|---|---|
| a, abc | Literal characters and concatenations | cat |
| `a | b` | Alternation (OR) |
| *, +, ? | Zero-or-more, One-or-more, Optional | a*, b+, colou?r |
| {n}, {n,m}, {n,} | Exact, bounded, or unbounded repetition | \d{4}, [a-z]{2,4} |
| [a-z], [^0-9] | Character classes and negated character classes | [A-Za-z0-9_] |
| \d, \w, \s | Shorthand digits (0-9), word characters, whitespace | \w+ |
| \n, \r, \t, \\ | Escape sequences | \t+ |
| (...) | Grouping for operator precedence | (ab)+ |
Quick Start
1. Generating a Tokenizer
Include the header, register your regex rules with associated Token IDs, and generate the source code for your tokenizer:
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
        { "\\d+(\\.\\d+)?",       NUMBER,     false },
        { "==|!=|\\+|\\-|\\*|/",   OPERATOR,   false },
        { "\\s+",                 WHITESPACE, false }
    });

    // Generate C++ code for a class named "Lexer"
    std::string code = compiler.generateCppClass("Lexer");

    // Output code to a header file
    std::ofstream out("Lexer.hpp");
    out << code;
    out.close();

    std::cout << "Lexer header generated successfully!\n";
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
                  << " | Lexeme: \"" << token.lexeme << "\""
                  << " | Pos: " << token.position << "\n";
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
