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
    std::cerr << " ---EXPR> ";
    for (auto b = token; b != end; b++) {
        std::cerr << *b;
    }
#endif
    std::shared_ptr<ExpressionAST> base = nullptr, current = nullptr, temp;

    while (token != end) {
        temp = nullptr;
        switch (token->type) {
            case TOK_VARIABLE:      // Variables may be passed in from assignments.
            case TOK_IDENTIFIER:    temp = std::make_shared<IdentifierAST>(token->s_val, depth); break;
            case TOK_STRING:        temp = std::make_shared<    StringAST>(token->s_val, depth); break;
            case TOK_BINARY:        temp = std::make_shared<    BinaryAST>(token->s_val, depth); break;
            case TOK_VALUE: {
                    if (token->s_val == std::string(1, LEX_DECIMAL_POINT)) {
                        temp = std::make_shared<DoubleAST>(token->d_val, depth);
                        
                    } else if (token->s_val == LEX_NULL) {
                        temp = std::make_shared<NullAST>(depth);
                        
                    } else {
                        temp = std::make_shared<LongAST>(token->l_val, depth);
                        
                    }
                    break;
                }

            case TOK_OPERATOR: {
                    // Negate begin/end is sugar for 0..-
                    if (token->s_val == std::string(1, LEX_NEGATE_BEGIN)) {
                        temp = std::make_shared<LongAST>(0L, depth);
                        
                    } else if (token->s_val == std::string(1, LEX_NEGATE_END)) {
                        temp = std::make_shared<OperatorAST>(std::string(1, LEX_SUB), false, depth);
                        
                    } else if (token->s_val == std::string(1, LEX_REFERENCE_BEGIN)) {
                        auto temp2 = std::make_shared<ReferenceAST>(token->targets_self, depth);
                        TOKEN_IT stop = end - 1;
                        while (stop > token && (stop->type != TOK_OPERATOR || stop->s_val != std::string(1, LEX_REFERENCE_END))) {
                            stop--;
                        }
                        if (stop - token < 2) {
                            throw std::domain_error("Malformed reference.");
                        }
                        auto ref_ex = parse_expression(token + 1, stop, depth);
                        temp2->add_expression(ref_ex);
                        temp = temp2;
                        token = stop;

                    } else {
                        temp = std::make_shared<  OperatorAST>(token->s_val, token->targets_self, depth);
                        
                    }
                    break;
                }

            default:
                throw std::domain_error("Unknown expression token: " + std::string(1, token->type) + " - this is a bug");
        }
        if (temp) {
            if (current) {
                current->set_next(temp);
                current = temp;
            } else {
                base = temp;
                current = temp;
                depth = -1;
            }
        }
        token++;
    }

    return base;
}


#define FN( NAME ) AST_PTR NAME(TOKEN_IT token, TOKEN_IT end, AST_PTR ast)


FN( parse_conditional) {
    // when_true and when_false will be identified during semantic analysis.
    if (end - token < 3) {
        throw std::domain_error("Empty conditional");
    }
    auto c = std::make_shared<ConditionalAST>((end - 1)->s_val, token->l_val);
    auto ex = parse_expression(token + 1, end - 1, token->l_val + 8);
    c->add_expression(ex);
    return ScopeAST::add_member(ast, c, false);
}

// An alias is effectively a named expression.
FN( parse_alias ) {
    if (end - token < 5) {
        throw std::domain_error("Malformed alias statement");
    }
    if ((end - 1)->type != TOK_ALIAS_END) {
        throw std::domain_error("Unclosed alias");
    }
    if ((end - 2)->type != TOK_IDENTIFIER) {
        throw std::domain_error("Unnamed alias");
    }
    auto a = std::make_shared<AliasAST>((end - 2)->s_val, token->l_val);
    auto ex = parse_expression(token + 2, end - 2, token->l_val + 8);
    a->add_expression(ex);
    return ScopeAST::add_member(ast, a, true);
}


FN (parse_function) {
    // Varargs will have been mis-labelled self-contents as they look the same
    // without the appropriate context.
    auto takes_varargs = (token + 1)->type == TOK_OPERATOR && (token + 1)->s_val == LEX_VARARGS_CONTENTS;
    
    if ((end - 1)->s_val.empty()) {
        throw std::domain_error("Irrecoverable error: unnamed function");
    }
    
    auto f = std::make_shared<FunctionAST>(
        (end - 1)->s_val,
        token->l_val,
        (end - 1)->is_static,
        takes_varargs
    );
    ast = ScopeAST::add_member(ast, f, true);

    // Check for parameters.
    end -= 2;
    while (end > token) {
        if (end->type == TOK_VARIABLE) {
            f->add_parameter(end->s_val);
        }
        end--;
    }
    return ast;
}


FN( parse_variable ) {
    if ((token + 1)->s_val.empty()) {
        throw std::domain_error("Irrecoverable error: unnamed variable");
    }
    auto v = std::make_shared<VariableAST>((token + 1)->s_val, token->l_val, (token + 1)->is_static);
    ast = ScopeAST::add_member(ast, v, true);

    // Check for assignment
    if (end - token >= 4 && (end - 1)->is_assignment()) {
        auto ex = parse_expression(token + 1, end, token->l_val + 8);
        ast = ScopeAST::add_member(ast, ex, false);
    }
    return ast;
}


FN( parse_else ) {
    // Elses are generated jump targets.
    auto a = std::make_shared<LabelAST>("auto_else_" + std::to_string(token->line_no), token->l_val);
    return ScopeAST::add_member(ast, a, true);
}


FN( parse_label ) {
    int depth = token->l_val;
    token++;
    
    if (token == end) {
        throw std::domain_error("Unexpected EOF while parsing label");
    }
    if (token->s_val.empty()) {
        throw std::domain_error("Irrecoverable error: unnamed label");
    }
    auto a = std::make_shared<LabelAST>(token->s_val, depth);
    return ScopeAST::add_member(ast, a, true);
}


FN( parse_scope ) {
#ifdef DEBUG
    for (auto z = token; z < end; z++) {
        std::cerr << *z;
    }
#endif

    // Determine what kind of scope we are looking at.
    if ((token + 1)->type == TOK_ALIAS_BEGIN) {
        ast = parse_alias(token, end, ast);

    // Function declarations are checked before variable declarations as
    // function declarations may begin with parameter declarations.
    } else if ((end - 1) >= token && (end - 1)->type == TOK_FUNCTION) {
        ast = parse_function(token, end, ast);

    } else if ((token + 1)->type == TOK_VARIABLE) {
        ast = parse_variable(token, end, ast);

    } else if ((token + 1)->type == TOK_LABEL) {
        ast = parse_label(token, end, ast);

    } else if ((token + 1)->type == TOK_OPERATOR && (token + 1)->s_val == std::string(1, LEX_ELSE)) {
        ast = parse_else(token, end, ast);

    } else if ((end - 1)->is_conditional()) {
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
        if (token->type != TOK_SCOPE) {
            throw std::domain_error("Not yet processing token " + std::to_string(token->type));
            token++;
            
        } else {
            
            // Find the end of the scope.
            auto scope_end = token + 1;
            while (scope_end != end && scope_end->type != TOK_SCOPE) {
                scope_end++;
            }
            ast = parse_scope(token, scope_end, ast);
            token = scope_end;
        }
    }
#ifdef DEBUG
    std::cerr << std::endl;
#endif

    return ast;
}
