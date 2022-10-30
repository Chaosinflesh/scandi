// Scandi: parser.cpp
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#include <iostream>
#include <iterator>
#include <vector>
#include "ast.h"
#include "globals.h"
#include "lexer.h"
#include "parser.h"


#define TOKEN_IT            std::vector<Token>::iterator
#define FN( NAME )          SHARED(AST) NAME(TOKEN_IT token, TOKEN_IT end,  SHARED(AST) parent)
#define EX( TOK )           "e_" + TOK->filename + "_" + std::to_string(TOK->line_no) + "_" + std::to_string(TOK->l_val)
#define CD( COND, TOK )     COND + "_" + TOK->filename + "_" + std::to_string(TOK->line_no) + "_" + std::to_string(TOK->l_val)

// parent passed into this is an already setup AST ready for the expression to
// be put into NEXT.
FN( parse_expression ) {
    if (debug_set) {
        std::cerr << " ---EXPR> ";
        for (auto b = token; b != end; b++) {
            std::cerr << *b;
        }
    }
    // This will start off as a nullptr, declaring for typing.
    auto next = parent->next;
    auto item = parent->next;

    while (token != end) {
        item = nullptr;
        TOKEN_IT ref_end = token;
        if (token->type == TOK_VARIABLE || token->type == TOK_IDENTIFIER) {
            item = SHARE(AST, AST_IDENTIFIER, token->s_val, parent->depth, false);
        } else if (token->type == TOK_STRING) {
            item = SHARE(AST, AST_STRING, token->s_val, parent->depth, false);
        } else if (token->type == TOK_BINARY) {
            item = SHARE(AST, AST_BINARY, token->s_val, parent->depth, false);
        } else if (token->type == TOK_VALUE && token->s_val == CHAR_STR(LEX_DECIMAL_POINT)) {
            item = SHARE(AST, AST_DOUBLE, "", parent->depth, false);
            item->numeric_value.d = token->d_val;
        } else if (token->type == TOK_VALUE && token->s_val == LEX_NULL) {
            item = SHARE(AST, AST_NULL, "", parent->depth, false);
        } else if (token->type == TOK_VALUE) {
            item = SHARE(AST, AST_LONG, "", parent->depth, false);
            item->numeric_value.l = token->l_val;
        } else if (token->type == TOK_OPERATOR && token->s_val == CHAR_STR(LEX_NEGATE_BEGIN)) {
            // Negate begin/end is sugar for 0 ... -
            item = SHARE(AST, AST_LONG, "", parent->depth, false);
            item->numeric_value.l = 0L;
        } else if (token->type == TOK_OPERATOR && token->s_val == CHAR_STR(LEX_NEGATE_END)) {
            item = SHARE(AST, AST_OPERATOR, CHAR_STR(LEX_SUB), parent->depth, false);
        } else if (token->type == TOK_OPERATOR && token->s_val == CHAR_STR(LEX_REFERENCE_BEGIN)) {
            item = SHARE(AST, AST_OPERATOR, CHAR_STR(LEX_REFERENCE_BEGIN), parent->depth, false);
            ref_end = end - 1;
            while (ref_end > token && (ref_end->type != TOK_OPERATOR || ref_end->s_val != CHAR_STR(LEX_REFERENCE_END))) {
                ref_end--;
            }
            if (ref_end - token < 2) {
                DERR("Malformed reference");
            }
            // The reference will be added after we've moved the item into next.
        } else if (token->type == TOK_OPERATOR) {
            item = SHARE(AST, AST_OPERATOR, token->s_val, parent->depth, false);
        } else {
            DERR("Unknown expression token: " + CHAR_STR(token->type));
        }

        // Apply the next item.
        if (token->targets_self) {
            item->set_property(AST::OPT_TARGETS_SELF);
        }
        if (!next) {
            parent->next = std::move(item);
            next = parent->next;
        } else {
            next->next = std::move(item);
            next = next->next;
        }

        // If we need to add a sub-expression. Gets added to ->alt so as to not pollute ->next space.
        if (ref_end != token) {
            auto exp = SHARE(AST, AST_REFERENCE, "ref_" + std::to_string(token->pos), parent->depth, false);
            next->alt = std::move(exp);
            parse_expression(token + 1, ref_end, next->alt);
            token = ref_end;
        }
        token++;
    }

    return parent;
}


FN( parse_conditional) {
    // when_false will be identified during semantic analysis.
    if (end - token < 3) {
        throw domain_error("Empty conditional");
    }
    auto name = CD((end - 1)->s_val, token);
    auto c = SHARE(AST, AST_CONDITIONAL, name, token->l_val, false);
    parent = AST::get_correct_parent(c, parent);
    c->parent = parent;
    parent->children.push_back(std::move(c));
    return parse_expression(token + 1, end - 1, parent->get_member(name));
}


// An alias is effectively a named expression.
FN( parse_alias ) {
    if (end - token < 5) {
        throw domain_error("Malformed alias statement");
    }
    if ((end - 1)->type != TOK_ALIAS_END) {
        throw domain_error("Unclosed alias");
    }
    if ((end - 2)->type != TOK_IDENTIFIER) {
        throw domain_error("Unnamed alias");
    }
    auto a = SHARE(AST, AST_ALIAS, (end - 2)->s_val, token->l_val, false);
    parent = AST::get_correct_parent(a, parent);
    a->parent = parent;
    parent->children.push_back(std::move(a));
    return parse_expression(token + 2, end - 2, parent->get_member((end - 2)->s_val));
}


FN (parse_function) {
    // Varargs will have been mis-labelled self-contents as they look the same
    // without the appropriate context.
    auto takes_varargs = (token + 1)->type == TOK_OPERATOR && (token + 1)->s_val == LEX_VARARGS_CONTENTS;
    
    if ((end - 1)->s_val.empty()) {
        throw domain_error("Irrecoverable error: unnamed function");
    }

    auto f = SHARE(AST, AST_FUNCTION, (end - 1)->s_val, token->l_val, (end - 1)->is_static);
    if (takes_varargs) {
        f->set_property(AST::OPT_HAS_VARARGS);
    }
    parent = AST::get_correct_parent(f, parent);
    f->parent = parent;
    parent->children.push_back(std::move(f));
    auto function = parent->get_member((end - 1)->s_val);

    // Check for parameters.
    end -= 2;
    auto next = function->next;
    while (end > token) {
        if (end->type == TOK_VARIABLE) {
            // Build the parameter declaration.
            auto p_name = end->s_val;
            auto p = SHARE(AST, AST_VARIABLE, p_name, token->l_val + 1, false);
            p->parent = function;

            // Allocate into ->next.
            if (!next) {
                function->next = std::move(p);
                next = function->next;
            } else {
                next->next = std::move(p);
                next = next->next;
            }
        }
        end--;
    }

    return function;
}


FN( parse_variable ) {
    if ((token + 1)->s_val.empty()) {
        throw domain_error("Irrecoverable error: unnamed variable");
    }
    auto v = SHARE(AST, AST_VARIABLE, (token + 1)->s_val, token->l_val, (token + 1)->is_static);
    parent = AST::get_correct_parent(v, parent);
    v->parent = parent;
    parent->children.push_back(std::move(v));

    // Check for assignment
    if (end - token >= 4 && (end - 1)->is_assignment()) {
        auto e_name = EX( token );
        auto e = SHARE(AST, AST_EXPRESSION, e_name, token->l_val, false);
        e->parent = parent;
        parent->children.push_back(std::move(e));
        parent = parse_expression(token + 1, end, parent->get_member(e_name));
    }
    return parent;
}


FN( parse_else ) {
    // Elses are generated jump targets.
    auto a = SHARE(AST, AST_LABEL, "auto_else_" + std::to_string(token->line_no), token->l_val, false);
    parent = AST::get_correct_parent(a, parent);
    a->parent = parent;
    parent->children.push_back(std::move(a));
    return parent;
}


FN( parse_label ) {
    int depth = token->l_val;
    token++;
    
    if (token == end) {
        throw domain_error("Unexpected EOF while parsing label");
    }
    if (token->s_val.empty()) {
        throw domain_error("Irrecoverable error: unnamed label");
    }
    auto a = SHARE(AST, AST_LABEL, token->s_val, depth, true);
    parent = AST::get_correct_parent(a, parent);
    a->parent = parent;
    parent->children.push_back(std::move(a));
    return parent;
}


FN( parse_raw ) {
    int depth = token->l_val;
    token++;
    auto r = SHARE(AST, AST_RAW, token->s_val, depth, false);
    parent = AST::get_correct_parent(r, parent);
    r->parent = parent;
    parent->children.push_back(std::move(r));
    return parent;
}


FN( parse_scope ) {
    if (debug_set) {
        for (auto z = token; z < end; z++) {
            std::cerr << *z;
        }
    }

    // Determine what kind of scope we are looking at.
    if ((token + 1)->type == TOK_RAW) {
        parent = parse_raw(token, end, parent);
        
    } else if (!token->s_val.empty()) {
        // This is a named scope.
        auto s = SHARE(AST, AST_SCOPE, token->s_val, token->l_val, true);
        parent = AST::get_correct_parent(s, parent);
        s->parent = parent;
        parent->children.push_back(std::move(s));
        
    } else if ((token + 1)->type == TOK_ALIAS_BEGIN) {
        parent = parse_alias(token, end, parent);

    // Function declarations are checked before variable declarations as
    // function declarations may begin with parameter declarations.
    } else if ((end - 1) >= token && (end - 1)->type == TOK_FUNCTION) {
        parent = parse_function(token, end, parent);

    } else if ((token + 1)->type == TOK_VARIABLE) {
        parent = parse_variable(token, end, parent);

    } else if ((token + 1)->type == TOK_LABEL) {
        parent = parse_label(token, end, parent);

    } else if ((token + 1)->type == TOK_OPERATOR && (token + 1)->s_val == std::string(1, LEX_ELSE)) {
        parent = parse_else(token, end, parent);

    } else if ((end - 1)->is_conditional()) {
        parent = parse_conditional(token, end, parent);

    } else if (end - token > 1) {
        auto e_name = EX( token );
        auto e = SHARE(AST, AST_EXPRESSION, e_name, token->l_val, false);
        parent = AST::get_correct_parent(e, parent);
        e->parent = parent;
        parent->children.push_back(std::move(e));
        parent = parse_expression(token + 1, end, parent->get_member(e_name));
    }

    return parent;
}


SHARED(AST) parse_to_ast(std::vector<Token> tokens,  SHARED(AST) ast) {
    DEBUG( "PARSING"; )
    auto token = tokens.begin();
    auto end = tokens.end();

    while (token != end) {
        if (token->type != TOK_SCOPE) {
            throw domain_error("Not yet processing token " + std::to_string(token->type));
            token++;
            
        } else {
            
            // Find the end of the scope. Need to account for named scopes (such
            // as files!)
            auto scope_end = token + 1;
            while (
                scope_end != end
             && (scope_end->type != TOK_SCOPE || !scope_end->s_val.empty())
            ) {
                scope_end++;
            }
            ast = parse_scope(token, scope_end, ast);
            token = scope_end;
        }
    }
    DEBUG( ""; )

    return ast;
}
