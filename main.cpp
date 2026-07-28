#pragma once
// written by Paul Baxter

#include <format>
#include <iostream>
#include <string>
#include <stdio.h>
#include <fstream>

#include "RegexEngine.h"

int main() {
    RegexCompiler compiler;

	compiler.addRules({
		{ "gr(e|a)+y", 1, true },       // Identifiers
		{ "balls?",  2, true },        	// Integers
		{ "id_\\w*",  3, true },  		// Custom rule
		{ "[ \\t\\n\\r]+", -1 }        	// Whitespace (skip/ignore)
	});
	
	std::string file = "parser.h"; 
	std::string classname = "Tokenizer";

    // Generate the C++ class string
    std::string code = compiler.generateCppClass(classname);

    // Save to a header file
    std::ofstream out(file);
    out << code;
    out.close();

    std::cout << classname << " class successfully generated in " << file << "!\n";
    return 0;
}
