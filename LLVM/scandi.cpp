// Scandi: scandi.cpp
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#include <iostream>
#include <string>
#include "ast.h"
#include "lexer.h"
#include "parser.h"


int main(int argc, char** argv) {
    std::vector<Token> tokens;
    // 1. Tokenize
    if (!tokenize_stream(tokens, std::cin, "stdin")) {
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
    auto global = std::make_shared<ScopeAST>(ScopeAST("global", AST_SCOPE, -1, true));
    auto file = std::make_shared<ScopeAST>(ScopeAST("file", AST_SCOPE, 0, true));
    auto ast = ScopeAST::add_member(global, file);
    if (!parse_to_ast(tokens, ast)) {
        std::cerr << "PARSING FAILED" << std::endl;
    }
//#ifdef DEBUG
    std::cout << *global << std::endl;
//#endif
    // 3. Semantic analysis
    //    TODO: Add in stdlib here
    return 0;
}
