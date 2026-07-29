#include <iostream>
#include "C64Tokenizer.hpp"

int main() {
    std::string source_code = "10 PRINT \"HELLO WORLD\"\n20 GOTO 10";

    // Call the generated static function
    std::vector<C64Tokenizer::Token> tokens = C64Tokenizer::tokenize(source_code);

    // Process tokens
    for (const auto& tok : tokens) {
        if (tok.id == -1) {
            std::cerr << "Lexical Error at line " << tok.line 
                      << ", col " << tok.column 
                      << ": Unexpected character '" << tok.lexeme << "'\n";
            continue;
        }

        std::cout << "Token ID: " << tok.id 
                  << " | Lexeme: [" << tok.lexeme << "]"
                  << " | Line: " << tok.line 
                  << " | Col: " << tok.column << "\n";
    }

    return 0;
}
