/*
 * scandi.cpp
 * ==========
 * By Neil Bradley
 * 
 * This should integrate with LLVM to create a compiler for scandi.
 */
#include <iostream>
#include <string>
#include "lexer.h"

#define DEBUG

int main(int argc, char** argv) {
	std::vector<Token> tokens;
	// 1. Tokenize
	if (!tokenizeStream(tokens, std::cin, "stdin")) {
		std::cerr << "LEXING FAILED" << std::endl;
		return 1;
	}
#ifdef DEBUG
	std::cout << tokens.size() << " tokens" << std::endl;
	for (auto t: tokens) {
		std::cout << t << std::endl;
	}
#endif
	
	// 2. Parse
	return 0;
}
