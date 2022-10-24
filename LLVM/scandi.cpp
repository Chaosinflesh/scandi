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

    // 2. Parse
    auto global = std::make_shared<ScopeAST>("global", -1, true);
    if (!parse_to_ast(tokens, global)) {
        std::cerr << "PARSING FAILED" << std::endl;
    }

    // 3. Semantic analysis
    //    TODO: Add in stdlib here
    return 0;
}
