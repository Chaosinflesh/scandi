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

    // If this is an expression, probably need to factor in -> next, as well!
    for (auto m : ast->members_by_order) {
        if (m->type == AST_IDENTIFIER) {
            auto start = std::dynamic_pointer_cast<ExpressionAST>(m);
            auto end = start;
            while (end->next) {
                end = end->next;
            }
            if (end->type == AST_OPERATOR) {
                auto op = std::dynamic_pointer_cast<OperatorAST>(end);
                if (op->op != std::string(1, LEX_ASSIGNMENT)) {
                    std::cout << std::endl << "Auto expanding: " << start->name << " " << op->op;
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

bool map_aliases(AST_PTR ast) {
    DEBUG( "TODO: Map aliases"; )
    return true;
}

bool map_identifiers(AST_PTR ast) {
    DEBUG( "TODO: Map identifiers."; )
    return true;
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

bool analyse_static_use(AST_PTR ast) {
    DEBUG( "TODO: statics."; )
    return true;
}

bool check_argument_counts(AST_PTR ast) {
    DEBUG( "TODO: Auto assignments."; ) 
    return true;
}

AST_PTR analyse_semantics(AST_PTR ast) {
    DEBUG( "ANALYSING SEMANTICS: TODO"; )
    bool success = true;
    success &= analyse_auto_assignments(ast);
    success &= map_aliases(ast);
    success &= map_identifiers(ast);
    success &= analyse_static_use(ast);
    success &= check_argument_counts(ast);
    success &= connect_conditionals(ast);
    DEBUG( ""; )
    if (!success) {
        throw std::domain_error("Semantic errors occured, please check the log");
    }
    return ast;
}
