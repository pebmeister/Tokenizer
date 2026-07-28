#include <iostream>
#include <string>
#include <string_view>
#include "parser.h" // Contains the generated class

int main() {
    std::string text = "There are grey greeeey grAY people with balls. They have id_123";
    std::cout << "Input text:\n" << text << "\n\nOutput:\n";

    // Call the static tokenize() method on your generated class
    // (If you gave a different class name in generateCppClass, use that name here)
    std::vector<Tokenizer::Token> tokens = Tokenizer::tokenize(text);

    for (const auto& token : tokens) {
        if (token.id > 0) {
            // Matched token rule (e.g. ID 1, 2, 3...)
            std::cout << "[" << token.lexeme << " #ID:" << token.id << "]";
        } else {
            // Token ID -1 or 0 represents unmatched single characters / whitespace
            std::cout << token.lexeme;
        }
    }
    
    std::cout << "\n";
    return 0;
}
