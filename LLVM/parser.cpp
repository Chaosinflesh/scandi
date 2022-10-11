// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0
#include <iostream>
#include <iterator>
#include <vector>
#include "ast.h"
#include "lexer.h"
#include "parser.h"


// Forward declarations
std::shared_ptr<CitizenAST> parseStack(std::vector<Token> stack, std::shared_ptr<CitizenAST> parent, int depth);


std::shared_ptr<CitizenAST> parseLabelDeclaration(
    std::vector<Token> stack,
    std::shared_ptr<CitizenAST> parent,
    int depth
) {
    if (stack.size() > 1) {
        if (stack[1].type == TOK_IDENTIFIER_ALIAS) {
            std::string identifier = stack[1].sVal;
            auto stackAST = std::make_shared<DeclaredCitizenAST>(DeclaredCitizenAST(identifier, TOK_DECLARATION_LABEL, depth, parent));
            parent.get()->members.push_back(stackAST);
            return stackAST;
        } else {
            throw std::invalid_argument("PARSER: Expected label identifier, found something else. Aborting.");
        }
    } else {
        throw std::invalid_argument("PARSER: Missing label identifier. Aborting.");
    }
}


std::shared_ptr<CitizenAST> parseVariableDeclaration(
    std::vector<Token> stack,
    std::shared_ptr<CitizenAST> parent,
    int depth,
    bool isStatic
) {
    if (stack.size() > 1) {
        if (stack[1].type == TOK_IDENTIFIER_ALIAS) {
            std::string identifier = stack[1].sVal;
            auto stackAST = std::make_shared<DeclaredCitizenAST>(DeclaredCitizenAST(
                identifier,
                isStatic ? TOK_DECLARATION_VARIABLE_STATIC : TOK_DECLARATION_VARIABLE,
                depth,
                parent
            ));
            parent.get()->members.push_back(stackAST);
            
            // Check for initialization also.
            if (stack.size() > 2) {
                if (stack.size() >= 4 && stack.back().type == TOK_ASSIGNMENT) {
                    std::vector<Token> expr;
                    expr.insert(expr.begin(), stack.begin() + 1, stack.end());
                    return parseStack(expr, parent, depth);
                } else {
                    throw std::invalid_argument("PARSER: Malformed variable initialization. Aborting.");
                }
            }
            return stackAST;
        } else {
            throw std::invalid_argument("PARSER: Expected variable identifier, found something else. Aborting.");
        }
    } else {
        throw std::invalid_argument("PARSER: Missing variable identifier. Aborting.");
    }
}


std::shared_ptr<CitizenAST> parseStack(std::vector<Token> stack, std::shared_ptr<CitizenAST> parent, int depth) {
    // Determine type of stack
    switch (stack[0].type) {
        case TOK_DECLARATION_LABEL:             return parseLabelDeclaration(stack, parent, depth);
        case TOK_DECLARATION_VARIABLE:          return parseVariableDeclaration(stack, parent, depth, false);
        case TOK_DECLARATION_VARIABLE_STATIC:   return parseVariableDeclaration(stack, parent, depth, true);
        default: break;
    }
    std::cout << "NOT YET IMPLEMENTED: Stack [" << stack[0].type << "]" << std::endl;
    auto stackAST = std::make_shared<CitizenAST>(CitizenAST(depth, parent));
    parent.get()->members.push_back(stackAST);
    return stackAST;
}


bool parseToAST(std::vector<Token> tokens, std::shared_ptr<CitizenAST> ast) {
    bool success = true;
    auto currentToken = tokens.begin();
    auto lastToken = tokens.end();
    while (currentToken != lastToken) {
        // 1. Calculate depth and parent.
        int depth = ast.get()->depth;
        while (currentToken != lastToken && currentToken->type == TOK_SCOPE) {
            depth = (int)currentToken->lVal;
            currentToken = std::next(currentToken);
        }
        if (currentToken == lastToken) {
            break;
        }
        while (depth <= ast.get()->depth) {
            if (ast.get()->scope) {
                ast = ast.get()->scope;
            } else {
                throw std::range_error("PARSER attempted to access a null object. This is a bug.");
            }
        }
        
        // 2. Parse the stack
        std::vector<Token> stack;
        while (currentToken != lastToken && currentToken->type != TOK_SCOPE) {
            stack.push_back(*currentToken);
            currentToken = std::next(currentToken);
        }
        ast = parseStack(stack, ast, depth);
    }
    return success;
}
