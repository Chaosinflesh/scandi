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
    // Sanity
    if (stack.empty()) {
        throw std::range_error("Empty stack. This is a bug!");
    }
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
    auto exprAST = std::make_shared<ExpressionAST>(ExpressionAST(depth, parent));
    auto current = stack.begin();
    std::vector<TokenType> notImplemented;
    while (current != stack.end()) {
        auto type = current->type;
        if (type == TOK_IDENTIFIER_ALIAS) {
            // The type of these will need to be change during semantic analysis.
            exprAST->stack.push_back(std::make_shared<AliasAST>(AliasAST(current->sVal, depth, parent)));
        } else if (
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
         || type == TOK_DOT
         // TODO: REF does not belong here -> it generates a nexted Expression!
         || type == TOK_REF_START
         || type == TOK_REF_END
         || type == TOK_ASSIGNMENT
         || type == TOK_COUNT
         || type == TOK_CONTENTS
        ) {
            exprAST->stack.push_back(std::make_shared<OperatorAST>(OperatorAST(depth, type, parent)));
        } else if (
            type == TOK_EQ
         || type == TOK_LT
         || type == TOK_LTE
         || type == TOK_GT
         || type == TOK_GTE
        ) {
            exprAST->stack.push_back(std::make_shared<ComparatorAST>(ComparatorAST(depth, type, parent)));
        } else if (type == TOK_NULL) {
            exprAST->stack.push_back(std::make_shared<NullAST>(NullAST(depth, parent)));
        } else if (type == TOK_STRING) {
            exprAST->stack.push_back(std::make_shared<StringAST>(StringAST(current->sVal, depth, parent)));
        } else if (type == TOK_NUMBER_LONG) {
            exprAST->stack.push_back(std::make_shared<IntegerAST>(IntegerAST(current->lVal, depth, parent)));
        } else if (type == TOK_NUMBER_DOUBLE) {
            exprAST->stack.push_back(std::make_shared<FloatAST>(FloatAST(current->dVal, depth, parent)));
        } else {
            notImplemented.push_back(type);
        }
        current++;
    }
    if (!notImplemented.empty()) {
        std::cout << "NOT YET IMPLEMENTED:";
        for (auto t : notImplemented) {
            std::cout << " " << t;
        }
        std::cout << std::endl;
    }
    parent.get()->members.push_back(exprAST);
    return exprAST;
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
