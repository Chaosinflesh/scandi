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
            auto stackAST = std::make_shared<DeclaredCitizenAST>(DeclaredCitizenAST(
                identifier,
                TOK_DECLARATION_LABEL,
                depth,
                false,
                parent
            ));
            parent.get()->members.push_back(stackAST);
            return stackAST;
        } else {
            throw std::domain_error("Expected label identifier, found something else. Aborting.");
        }
    } else {
        throw std::domain_error("Missing label identifier. Aborting.");
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
                TOK_DECLARATION_VARIABLE,
                depth,
                isStatic,
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
                    throw std::domain_error("Malformed variable initialization. Aborting.");
                }
            }
            return stackAST;
        } else {
            throw std::domain_error("Expected variable identifier, found something else. Aborting.");
        }
    } else {
        throw std::domain_error("Missing variable identifier. Aborting.");
    }
}


std::shared_ptr<CitizenAST> parseFunctionDeclaration(
    std::vector<Token> stack,
    std::shared_ptr<CitizenAST> parent,
    int depth,
    bool isStatic
) {
    if (stack.back().type == TOK_IDENTIFIER_ALIAS) {
        std::string identifier = stack.back().sVal;
        auto functionAST = std::make_shared<FunctionAST>(FunctionAST(
            identifier,
            isStatic,
            stack[0].type == TOK_VARARGS,
            depth,
            parent
        ));
        
        // Parse argument declarations
        auto argument = stack.begin();
        if (argument->type == TOK_VARARGS) {
            argument++;
        }
        while (argument != stack.end() - 2) {
            if (argument->type == TOK_DECLARATION_VARIABLE && (argument + 1)->type == TOK_IDENTIFIER_ALIAS) {
                std::vector<Token> varExpr;
                varExpr.insert(varExpr.begin(), argument, argument + 2);
                functionAST->arguments.insert(
                    functionAST->arguments.begin(),
                    parseVariableDeclaration(varExpr, functionAST, depth + 1, false)
                );
            } else {
                throw std::domain_error("Malformed variable declaration in function definition.");
            }
            argument += 2;
        }
        
        parent.get()->members.push_back(functionAST);
        return functionAST;
    } else {
        throw std::domain_error("Expected label identifier, found something else. Aborting.");
    }
}


std::shared_ptr<CitizenAST> parseExpression(
    std::vector<Token> stack,
    std::shared_ptr<CitizenAST> parent,
    int depth
) {
    // Check for operator shorthand (e.g. var ... op).
    if (stack[0].type == TOK_IDENTIFIER_ALIAS) {
        TokenType type = stack.back().type;
        if (
            type == TOK_ADD
         || type == TOK_SUBTRACT
         || type == TOK_MULTIPLY
         || type == TOK_DIVIDE
         || type == TOK_MODULUS
         || type == TOK_AND
         || type == TOK_OR
         || type == TOK_XOR
         || type == TOK_COMPLEMENT
         || type == TOK_SHL
         || type == TOK_SHR
         || type == TOK_SSHR
        ) {
            stack.insert(stack.begin(), stack.front());
            stack.push_back(Token(TOK_ASSIGNMENT));
        }
    }
    std::cout << "NOT YET IMPLEMENTED: Expression [" << stack[0].type << "]" << stack.size() << " components: ";
    for (auto t: stack) {
        if (t.type == TOK_IDENTIFIER_ALIAS) {
            std::cout << " " << t.sVal;
        } else {
            std::cout << " " << t.type;
        }
    }
    std::cout << std::endl;
    auto stackAST = std::make_shared<CitizenAST>(CitizenAST(depth, false, parent));
    parent.get()->members.push_back(stackAST);
    return stackAST;
}


std::shared_ptr<CitizenAST> parseAlias(std::vector<Token> stack, std::shared_ptr<CitizenAST> parent, int depth) {
    if (stack.size() < 3 || stack.back().type != TOK_WITH_END) {
        throw std::domain_error("Empty or malformed with statement. Aborting.");
    }
    // Check for alias
    std::string alias;
    std::vector<Token> exprStack;
    auto size = stack.size();
    auto exprEnd = stack.end() - 1;
    if (stack[size -2].type == TOK_IDENTIFIER_ALIAS) {
        alias = stack[size - 2].sVal;
    }
    if (stack[size - 3].type == TOK_IDENTIFIER_ALIAS) {
        exprEnd--;
    }
    exprStack.insert(exprStack.begin(), stack.begin() + 1, exprEnd);
    
    // Sanity check.
    if (alias.empty() || exprStack.empty()) {
        throw std::domain_error("Malformed with statement.");
    }
    
    auto aliasAST = std::make_shared<AliasAST>(AliasAST(alias, depth, parent));
    parseExpression(exprStack, aliasAST, depth);
    parent.get()->members.push_back(aliasAST);
    
    return parent;
}


std::shared_ptr<CitizenAST> parseStack(std::vector<Token> stack, std::shared_ptr<CitizenAST> parent, int depth) {
    // Determine type of stack
    if (stack.size() >= 2) {
        // This needs to be first as function declaration may include variable declarations.
        if ((stack.end() - 2)->type == TOK_DECLARATION_FUNCTION) {
            return parseFunctionDeclaration(stack, parent, depth, false);

        } else if ((stack.end() - 2)->type == TOK_DECLARATION_FUNCTION_STATIC) {
            return parseFunctionDeclaration(stack, parent, depth, true);
        }
    }
    if (stack[0].type == TOK_DECLARATION_LABEL) {
        return parseLabelDeclaration(stack, parent, depth);
        
    } else if (stack[0].type == TOK_DECLARATION_VARIABLE) {
        return parseVariableDeclaration(stack, parent, depth, false);
        
    } else if (stack[0].type == TOK_DECLARATION_VARIABLE_STATIC) {
        return parseVariableDeclaration(stack, parent, depth, true);
        
    } else if (stack[0].type == TOK_WITH_START) {
        return parseAlias(stack, parent, depth);
        
    }
    
    return parseExpression(stack, parent, depth);
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
            currentToken++;
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
            currentToken++;
        }
        ast = parseStack(stack, ast, depth);
    }
    return success;
}
