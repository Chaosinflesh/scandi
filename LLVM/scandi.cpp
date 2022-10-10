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

int main(int argc, char** argv) {
	std::vector<Token> tokens;
	for (std::string line; std::getline(std::cin, line); ) {
		tokenizeLine(tokens, line);
	}
	std::cout << tokens.size() << " tokens" << std::endl;
	for (auto t: tokens) {
		std::cout << t << std::endl;
	}
	return 0;
}
