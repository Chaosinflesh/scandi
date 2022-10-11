// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0
#include <iostream>
#include <string>
#include "ast.h"
#include "lexer.h"
#include "parser.h"

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
    auto ast = std::make_shared<CitizenAST>(CitizenAST(0, std::shared_ptr<CitizenAST>(nullptr)));
    if (!parseToAST(tokens, ast)) {
        std::cerr << "PARSING FAILED" << std::endl;
    }
#ifdef DEBUG
    std::cout << *ast << std::endl;
#endif

    // 3. Semantic analysis
    //    TODO: Add in stdlib here
    return 0;
}
