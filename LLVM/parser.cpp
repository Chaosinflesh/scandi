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
#define EXP_PTR std::shared_ptr<ExpressionAST>


EXP_PTR parse_expression(TOKEN_IT token, TOKEN_IT end, int depth) {
#ifdef DEBUG
    std::cerr << " parsing expression:";
    for (auto b = token; b != end; b++) {
        std::cerr << " " << b->type;
    }
#endif
    auto ex = std::make_shared<ExpressionAST>(depth);
    
    // TODO: Add sub-expressions here!
    
    return ex;
}


AST_PTR parse_conditional(TOKEN_IT token, TOKEN_IT end, AST_PTR ast) {
    // when_true and when_false will be identified during semantic analysis.
    if (end - token < 3) {
        throw std::domain_error("Empty conditional");
    }
    auto c = std::make_shared<ConditionalAST>(ConditionalAST(token->l_val));
    auto ex = parse_expression(token + 1, end - 1, token->l_val + 8);
    c->add_expression(ex);
    return ScopeAST::add_member(ast, c, false);
}


// An alias is effectively a named expression.
AST_PTR parse_alias(TOKEN_IT token, TOKEN_IT end, AST_PTR ast) {
    if (end - token < 5) {
        throw std::domain_error("Malformed alias statement");
    }
    if ((end - 1)->type != TOK_ALIAS_END) {
        throw std::domain_error("Unclosed alias");
    }
    if ((end - 2)->type != TOK_IDENTIFIER) {
        throw std::domain_error("Unnamed alias");
    }
    auto a = std::make_shared<AliasAST>(AliasAST((end - 2)->s_val, token->l_val));
    auto ex = parse_expression(token + 2, end - 2, token->l_val + 8);
    a->add_expression(ex);
    return ScopeAST::add_member(ast, a, true);
}


AST_PTR parse_function(TOKEN_IT token, TOKEN_IT end, AST_PTR ast) {
    auto is_static = (end - 2)->type == TOK_FUNCTION_STATIC;
    // Varargs will have been mis-labelled self-contents as they look the same
    // without the appropriate context.
    auto takes_varargs = (token + 1)->type == TOK_SELF_CONTENTS;
    
    if ((end - 1)->type != TOK_IDENTIFIER) {
        throw std::domain_error("Irrecoverable error: unnamed function");
    }
    
    auto f = std::make_shared<FunctionAST>(FunctionAST(
        (end - 1)->s_val,
        token->l_val,
        is_static,
        takes_varargs
    ));
    ast = ScopeAST::add_member(ast, f, true);

    // Check for parameters.
    end -= 3;
    while (end > token) {
        if (end->type == TOK_IDENTIFIER) {
            f->add_parameter(end->s_val);
        }
        end--;
    }
    return ast;
}


AST_PTR parse_variable(TOKEN_IT token, TOKEN_IT end, AST_PTR ast) {
    auto is_static = (token + 1)->type == TOK_VARIABLE_STATIC;
    if (token + 2 == end || (token + 2)->type != TOK_IDENTIFIER) {
        throw std::domain_error("Irrecoverable error: unnamed variable");
    }
    auto v = std::make_shared<VariableAST>(VariableAST(
        (token + 2)->s_val,
        token->l_val,
        is_static
    ));
    ast = ScopeAST::add_member(ast, v, true);

    // Check for assignment
    if (end - token >= 5 && (end - 1)->type == TOK_ASSIGNMENT) {
        auto ex = parse_expression(token + 2, end, token->l_val);
        ast = ScopeAST::add_member(ast, ex, false);
    }
    return ast;
}


AST_PTR parse_else(TOKEN_IT token, TOKEN_IT end, AST_PTR ast) {
    // Elses are generated jump targets.
    auto a = std::make_shared<LabelAST>(LabelAST(
        "auto_else_" + std::to_string(token->line_no),
        token->l_val
    ));
    return ScopeAST::add_member(ast, a, true);
}


AST_PTR parse_label(TOKEN_IT token, TOKEN_IT end, AST_PTR ast) {
    int depth = token->l_val;
    token += 2;
    
    if (token == end) {
        throw std::domain_error("Unexpected EOF while parsing label");
    }
    if (token->type != TOK_IDENTIFIER) {
        throw std::domain_error("Irrecoverable error: unnamed label");
    }
    auto a = std::make_shared<LabelAST>(LabelAST(token->s_val, depth));
    return ScopeAST::add_member(ast, a, true);
}


AST_PTR parse_scope(TOKEN_IT token, TOKEN_IT end, AST_PTR ast) {
#ifdef DEBUG
    std::cerr << std::endl << "Parsing " << (end - token) << " tokens:\t";
    for (auto z = token; z < end; z++) {
        std::cerr << z->type << " ";
    }
#endif

    // Determine what kind of scope we are looking at.
    if ((token + 1)->type == TOK_ALIAS_BEGIN) {
        ast = parse_alias(token, end, ast);

    // Function declarations are checked before variable declarations as
    // function declarations may begin with parameter declarations.
    } else if ((end - 2 >= token) && ((end - 2)->type == TOK_FUNCTION_DECL || (end - 2)->type == TOK_FUNCTION_STATIC)) {
        ast = parse_function(token, end, ast);

    } else if ((token + 1)->type == TOK_VARIABLE_DECL || (token + 1)->type == TOK_VARIABLE_STATIC) {
        ast = parse_variable(token, end, ast);

    } else if ((token + 1)->type == TOK_LABEL_DECL) {
        ast = parse_label(token, end, ast);

    } else if ((token + 1)->type == TOK_ELSE) {
        ast = parse_else(token, end, ast);
    } else if (is_token_conditional((end -1)->type)) {
        ast = parse_conditional(token, end, ast);

    } else if (end - token > 1) {
        auto ex = parse_expression(token + 1, end, token->l_val);
        return ScopeAST::add_member(ast, ex, false);
    }
    
    return ast;
}


AST_PTR parse_to_ast(std::vector<Token> tokens, AST_PTR ast) {
    auto token = tokens.begin();
    auto end = tokens.end();

    while (token != end) {

        // Skip comments
        while (token->type == TOK_COMMENT) {
#ifdef DEBUG
            std::cerr << std::endl << "Comment: " << token->s_val;
#endif
            token++;
        }

        if (token->type != TOK_SCOPE) {
            std::cerr << "Not yet processing token " << token->type << std::endl;
            token++;
            
        } else {
            
            // Find the end of the scope.
            auto scope_end = token + 1;
            while (scope_end != end && scope_end->type != TOK_SCOPE && scope_end->type != TOK_COMMENT) {
                scope_end++;
            }
            ast = parse_scope(token, scope_end, ast);
            token = scope_end;
        }
    }

    return ast;
}
