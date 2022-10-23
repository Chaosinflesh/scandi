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
    std::shared_ptr<ExpressionAST> base = nullptr, current = nullptr, temp;

    while (token != end) {
        temp = nullptr;
        switch (token->type) {
            case TOK_IDENTIFIER:    temp = std::make_shared<IdentifierAST>(token->s_val, depth); break;
            case TOK_STRING:        temp = std::make_shared<    StringAST>(token->s_val, depth); break;
            case TOK_NUMBER_LONG:   temp = std::make_shared<      LongAST>(token->l_val, depth); break;
            case TOK_NEGATE_BEGIN:  temp = std::make_shared<      LongAST>(           0, depth); break;
            case TOK_NUMBER_DOUBLE: temp = std::make_shared<    DoubleAST>(token->d_val, depth); break;
            case TOK_NULL:          temp = std::make_shared<      NullAST>(              depth); break;
            case TOK_ADDRESS:       temp = std::make_shared<  OperatorAST>("_",   false, depth); break;
            case TOK_DOT:           temp = std::make_shared<  OperatorAST>(".",   false, depth); break;
            case TOK_SELF_DOT:      temp = std::make_shared<  OperatorAST>(".",    true, depth); break;
            case TOK_COUNT:         temp = std::make_shared<  OperatorAST>("!",   false, depth); break;
            case TOK_SELF_COUNT:    temp = std::make_shared<  OperatorAST>("!",    true, depth); break;
            case TOK_CONTENTS:      temp = std::make_shared<  OperatorAST>("[]",  false, depth); break;
            case TOK_SELF_CONTENTS: temp = std::make_shared<  OperatorAST>("[]",   true, depth); break;
            case TOK_COMPLEMENT:    temp = std::make_shared<  OperatorAST>("~",   false, depth); break;
            case TOK_ASSIGNMENT:    temp = std::make_shared<  OperatorAST>("=",   false, depth); break;
            case TOK_ADD:           temp = std::make_shared<  OperatorAST>("+",   false, depth); break;
            case TOK_SUB:           temp = std::make_shared<  OperatorAST>("-",   false, depth); break;
            case TOK_NEGATE_END:    temp = std::make_shared<  OperatorAST>("-",   false, depth); break;
            case TOK_MULTIPLY:      temp = std::make_shared<  OperatorAST>("*",   false, depth); break;
            case TOK_DIVIDE:        temp = std::make_shared<  OperatorAST>("/",   false, depth); break;
            case TOK_MODULUS:       temp = std::make_shared<  OperatorAST>("%",   false, depth); break;
            case TOK_AND:           temp = std::make_shared<  OperatorAST>("&",   false, depth); break;
            case TOK_OR:            temp = std::make_shared<  OperatorAST>("|",   false, depth); break;
            case TOK_XOR:           temp = std::make_shared<  OperatorAST>("^",   false, depth); break;
            case TOK_SHL:           temp = std::make_shared<  OperatorAST>("<-",  false, depth); break;
            case TOK_SHR:           temp = std::make_shared<  OperatorAST>("->",  false, depth); break;
            case TOK_SSHR:          temp = std::make_shared<  OperatorAST>(">>",  false, depth); break;
            case TOK_EQ:            temp = std::make_shared<  OperatorAST>("?",   false, depth); break;
            case TOK_LT:            temp = std::make_shared<  OperatorAST>("<",   false, depth); break;
            case TOK_LTE:           temp = std::make_shared<  OperatorAST>("?<",  false, depth); break;
            case TOK_GT:            temp = std::make_shared<  OperatorAST>(">",   false, depth); break;
            case TOK_GTE:           temp = std::make_shared<  OperatorAST>("?>",  false, depth); break;
            case TOK_REFERENCE_BEGIN: 
            case TOK_SELF_REFERENCE: {
                    auto temp2 = std::make_shared<ReferenceAST>(token->type == TOK_SELF_REFERENCE, depth);
                    TOKEN_IT stop = end;
                    while (stop > token && stop->type != TOK_REFERENCE_END) {
                        stop--;
                    }
                    if (stop - token < 2) {
                        throw std::domain_error("Malformed reference.");
                    }
                    auto ref_ex = parse_expression(token + 1, stop, depth);
                    temp2->add_expression(ref_ex);
                    temp = temp2;
                    token = stop;
                    break;
                }
            default:
                throw std::domain_error("Unknown expression token: " + std::to_string(token->type) + " - this is a bug");
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


AST_PTR parse_conditional(TOKEN_IT token, TOKEN_IT end, AST_PTR ast) {
    // when_true and when_false will be identified during semantic analysis.
    if (end - token < 3) {
        throw std::domain_error("Empty conditional");
    }
    std::string condition;
    switch ((end - 1)->type) {
        case TOK_EQ: condition = "?"; break;
        case TOK_LT: condition = "<"; break;
        case TOK_LTE: condition = "?<"; break;
        case TOK_GT: condition = ">"; break;
        case TOK_GTE: condition = "?>"; break;
        default:
            throw std::domain_error("Invalid condition detected " + std::to_string((end - 1)->type) + " this is a bug");
    }
    
    auto c = std::make_shared<ConditionalAST>(condition, token->l_val);
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
        auto ex = parse_expression(token + 2, end, token->l_val + 8);
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
    } else if (is_token_conditional((end -1 )->type)) {
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
            throw std::domain_error("Not yet processing token " + std::to_string(token->type));
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
#ifdef DEBUG
    std::cerr << std::endl;
#endif

    return ast;
}
