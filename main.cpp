#include <iostream>
#include <fstream>
#include "RegexEngine.h" // Header containing your NFABuilder, DFAConverter, and RegexCompiler

// 1. Define distinct Token IDs for your grammar
enum TokenType {
    TOKEN_KEYWORD    = 1,
    TOKEN_IDENTIFIER = 2,
    TOKEN_NUMBER     = 3,
    TOKEN_OPERATOR   = 4,
    TOKEN_STRING     = 5,
    TOKEN_WHITESPACE = 6
};

int main() {
    RegexCompiler compiler;

    // Option A: Single rule registration with flags
    // Pattern, Token ID, Case Insensitive, Anchor BOL, Anchor EOL
    compiler.addRule("rem.*", TOKEN_KEYWORD, true); 

    // Option B: Batch registration via addRules() initializer list
    compiler.addRules({
        // Keywords (Case-insensitive)
//        { "PRINT|GOTO|IF|THEN|FOR|NEXT", TOKEN_KEYWORD, true },

        // Identifiers & Numbers
//        { "[a-zA-Z_][a-zA-Z0-9_]*",     TOKEN_IDENTIFIER },
//        { "[0-9]+(\\.[0-9]+)?",          TOKEN_NUMBER },

        // Strings & Operators
//       { "\"([^\"]|\\\\\")*\"",         TOKEN_STRING },
//      { "\\+|\\-|\\*|\\/|=",           TOKEN_OPERATOR },

        // Whitespace (spaces, tabs, newlines)
        { "[ \\t\\r\\n]+",               TOKEN_WHITESPACE }
    });

    // 2. Generate the C++ code for the compiled DFA tokenizer
    std::string generated_code = compiler.generateCppClass("C64Tokenizer");

    // 3. Save the generated class to a standalone header file
    std::ofstream out("C64Tokenizer.hpp");
    out << generated_code;
    out.close();

    std::cout << "Successfully generated 'C64Tokenizer.hpp'!\n";
    return 0;
}
