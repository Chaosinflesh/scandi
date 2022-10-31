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
 *  This checks two basic things:
 *
 * 1. All items can see the global space.
 * 2. All identifiers are linked to their declarations.
 */

void check_for_global_access(const SHARED(AST) ast, const SHARED(AST) global) {
    DEBUG("CHECKING " << ast->name;)
    if (!ast->can_see(global->name)) {
        DERR("Could not see global namespace from " + ast->shorthand());
    }
    // Check each member.
    if (ast->next) {
        DEBUG(" NEXT ";)
        check_for_global_access(ast->next, global);
    }
    if (ast->alt) {
        DEBUG(" ALT ";)
        check_for_global_access(ast->alt, global);
    }
    for (auto c: ast->children) {
        check_for_global_access(c, global);
    }
}


void link_identifiers(SHARED(AST) ast) {
    if (ast->type == AST_IDENTIFIER) {
        DEBUG("LOOKING FOR " << ast->name;)
        auto link = ast->get_member(ast->name);
        if (link) {
            ast->alt = link;
        } else {
            DERR("Could not find member " + ast->name);
        }
    }
    // Check each member.
    if (ast->next) {
        DEBUG(" NEXT ";)
        link_identifiers(ast->next);
    }
    if (ast->type != AST_IDENTIFIER && ast->alt) {
        DEBUG(" ALT ";)
        link_identifiers(ast->alt);
    }
    for (auto c: ast->children) {
        link_identifiers(c);
    }
}


void analyse_semantics(SHARED(AST) ast) {
    DEBUG("CHECKING GLOBAL ACCESS CONSISTENCY (note this is for compiler debugging)";)
    check_for_global_access(ast, ast);
    DEBUG("LINKING IDENTIFIERS";)
    link_identifiers(ast);
}
