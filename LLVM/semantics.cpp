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
    DEBUG( "TODO: Auto assignments."; )
    return true;
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
