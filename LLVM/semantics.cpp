// Scandi: semantics.cpp
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
#include "semantics.h"

/*
 *  Semantics analysis has a number of tasks to do:
 *
 *  1. It needs to expand auto-assignments ( e.g. n 1 + ).
 *  2. It needs to replace/map aliases as possible.
 *  3. It needs to map (where possible) identifiers to declarations.
 *  4. It needs to link conditionals with their auto_else if present.
 *  5. It needs to check static access where possible.
 *  6. Check basic argument counts.
 *
 *  Note that it doesn't check stack validity.
 */


bool analyse_auto_assignments(AST_PTR ast) {
    if (ast->members_by_order.empty()) {
        return true;
    }
    bool success = true;

    // Only outer expressions ending in non-assignment can be auto-assigned.
    for (auto m : ast->members_by_order) {
        if (m->type == AST_IDENTIFIER) {
            auto start = std::dynamic_pointer_cast<ExpressionAST>(m);
            auto end = start;
            while (end->next) {
                end = end->next;
            }
            if (end->type == AST_OPERATOR) {
                auto op = std::dynamic_pointer_cast<OperatorAST>(end);
                if (op->name != std::string(1, LEX_ASSIGNMENT)) {
                    std::cout << std::endl << "Auto expanding: " << start->name << " " << op->name;
                    auto new_start = std::make_shared<IdentifierAST>(start->name, start->depth);
                    auto new_end = std::make_shared<OperatorAST>(std::string(1, LEX_ASSIGNMENT), false, end->depth);
                    new_start->next = start->next;
                    start->next = new_start;
                    end->next = new_end;
                }
            }
        }
        success &= analyse_auto_assignments(m);
    }
    return success;
}


bool connect_conditionals(AST_PTR ast) {
    if (ast->members_by_order.empty()) {
        return true;
    }
    bool success = true;
    auto current = ast->members_by_order.begin();
    auto end = ast->members_by_order.end();

    while (current != ast->members_by_order.end()) {
        success &= connect_conditionals(*current);
        if ((*current)->type == AST_CONDITIONAL) {
            if ((*current)->members_by_order.empty()) {
                std::cerr << std::endl << "Warning: conditional without success block";
            }
            auto next = current + 1;
            // A conditional without a trailing else, or even members, is ok.
            if (
                next != end
             && (*next)->type == AST_LABEL
             && (*next)->name.rfind("auto_else_", 0) == 0
            ) {
                DEBUG( "Switching items " << (*next)->name; )
                auto c = std::dynamic_pointer_cast<ConditionalAST>(*current);
                c->when_false = *next;
                current = ast->members_by_order.erase(next);
                continue;
            }
        }
        current++;
    }    
    return success;
}


void connect_labels(AST_PTR ast, int d, bool known) {
    if (!ast) {
        return;
    }
    DEBUG( std::string(d, ' ') << AST_PTR_INFO( ast ) << ast->name << (known ? " YES" : ""); )

    switch (ast->type) {
        // Expressions
        case AST_CONDITIONAL:
        case AST_ALIAS:
        case AST_EXPRESSION:
        case AST_IDENTIFIER:
        case AST_BINARY:
        case AST_STRING:
        case AST_LONG:
        case AST_DOUBLE:
        case AST_NULL:
        case AST_REFERENCE:
        case AST_OPERATOR: {
                auto e = std::dynamic_pointer_cast<ExpressionAST>(ast);
                // TODO: handle References' expression.
                if (ast->type == AST_CONDITIONAL) {
                    DEBUG( std::string(d, ' ') << "CONDITION:"; )
                }
                if (e->next){
                    connect_labels(e->next, d + 1, ast->has_member_visible(e->name));
                }
            }
            break;

        default: /* Already done above */ break;
    }

    if (ast->type == AST_CONDITIONAL) {
        DEBUG( std::string(d, ' ') << "TRUE"; )
    }
    // Applicable for all types.
    for (auto m: ast->members_by_order) {
        connect_labels(m, d + 2, ast->has_member_visible(m->name));
    }
    if (ast->type == AST_CONDITIONAL) {
        DEBUG( std::string(d, ' ') << "FALSE"; )
        auto c = std::dynamic_pointer_cast<ConditionalAST>(ast);
        if (c->when_false) {
            connect_labels(c->when_false, d + 2, ast->has_member_visible(c->when_false->name)); 
        }
        DEBUG( std::string(d, ' ') << "AFTER"; )
    }
}


AST_PTR analyse_semantics(AST_PTR ast) {
    DEBUG( "ANALYSING SEMANTICS: TODO"; )
    bool success = true;
    connect_labels(ast, 0, false);
    success &= analyse_auto_assignments(ast);
    success &= connect_conditionals(ast);
    DEBUG( ""; )
    if (!success) {
        throw domain_error("Semantic errors occured, please check the log");
    }
    return ast;
}
