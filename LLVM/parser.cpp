// Scandi: parser.cpp
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#include <iostream>
#include <iterator>
#include <vector>
#include "ast.h"
#include "lexer.h"
#include "parser.h"

#define TOKEN_IT std::vector<Token>::iterator
#define AST_PTR std::shared_ptr<ScopeAST>

struct token_ast {
    TOKEN_IT token;
    TOKEN_IT end;
    AST_PTR ast;

    token_ast(
        TOKEN_IT token,
        TOKEN_IT end,
        AST_PTR ast
    ) :
        token(token),
        end(end),
        ast(ast)
    {}
};


bool parse_function(token_ast& ta, bool is_static) {
    ta.token++;
    if (ta.token == ta.end) {
        throw std::domain_error("Unexpected EOF while parsing function");
    }
    if (ta.token->type != TOK_IDENTIFIER) {
        throw std::domain_error("Irrecoverable error: unnamed function");
    }
    auto l = std::make_shared<FunctionAST>(FunctionAST(
        ta.token->s_val,
        ta.ast->get_depth() + 1,
        is_static
    ));
    ScopeAST::add_member(ta.ast, l);
    return true;
}


bool parse_variable(token_ast& ta, bool is_static) {
    ta.token++;
    if (ta.token == ta.end) {
        throw std::domain_error("Unexpected EOF while parsing variable");
    }
    if (ta.token->type != TOK_IDENTIFIER) {
        throw std::domain_error("Irrecoverable error: unnamed variable");
    }
    auto l = std::make_shared<VariableAST>(VariableAST(
        ta.token->s_val,
        ta.ast->get_depth() + 1,
        is_static
    ));
    ScopeAST::add_member(ta.ast, l);
    return true;
}


bool parse_label(token_ast& ta) {
    ta.token++;
    if (ta.token == ta.end) {
        throw std::domain_error("Unexpected EOF while parsing label");
    }
    if (ta.token->type != TOK_IDENTIFIER) {
        throw std::domain_error("Irrecoverable error: unnamed label");
    }
    auto l = std::make_shared<LabelAST>(LabelAST(
        ta.token->s_val,
        ta.ast->get_depth() + 1,
        false
    ));
    ScopeAST::add_member(ta.ast, l);
    return true;
}


bool parse_scope(token_ast& ta) {
    auto a = std::make_shared<ScopeAST>(ScopeAST(
        ta.token->s_val.empty()
            ? "scope" + std::to_string(ta.token->line_no)
            : ta.token->s_val,
        ta.token->l_val,
        false
    ));
    ta.ast = ScopeAST::add_member(ta.ast, a);
    return true;
}


bool parse_to_ast(std::vector<Token> tokens, AST_PTR ast) {
    bool success = true;
    auto token = tokens.begin();
    auto end = tokens.end();
    
    while (token != end) {

        token_ast ta(token, end, ast);
        switch (token->type) {
            // Scope
            case TOK_SCOPE: success &= parse_scope(ta); break;
            case TOK_LABEL_DECL: success &= parse_label(ta); break;
            case TOK_VARIABLE_DECL: success &= parse_variable(ta, false); break;
            case TOK_VARIABLE_STATIC: success &= parse_variable(ta, true); break;
            case TOK_FUNCTION_DECL: success &= parse_function(ta, false); break;
            case TOK_FUNCTION_STATIC: success &= parse_function(ta, true); break;

            // Not yet implemented.
            default:
                std::cerr << "Not yet processing token " << token->type << std::endl;
                break;
        }
        token++;
    }

    return success;
}
