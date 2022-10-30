// Scandi: codegen.cpp
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#include "codegen.h"
#include "globals.h"


/*
 *  Initially, all I will be doing here is printing out the actions the program
 *  should take, as it encounters each AST object.
 */

void gen_scope(AST_PTR ast) {
    DEBUG( "NEW SCOPE " << ast->name; )
    // Add all the members recursively.
    for (auto m: ast->members_by_order) {
        generate_code(m);
    }
    DEBUG( "EXIT SCOPE " << ast->name << std::endl; )
}


void gen_raw(AST_PTR ast) {
    DEBUG( "<<< INJECT RAW LLVM IR >>>"; )
}


void gen_label(AST_PTR ast) {
    DEBUG( "ADD LABEL " << ast->name; )
    for (auto m: ast->members_by_order) {
        generate_code(m);
    }
}


void gen_variable(AST_PTR ast) {
    bool is_class = !ast->members_by_order.empty();
    DEBUG( "ADD " << (ast->is_static ? "STATIC " : "") << (is_class ? "CLASS " : "VARIABLE ") << ast->name; )
    if (is_class) {
        for (auto m: ast->members_by_order) {
            generate_code(m);
        }
        DEBUG( "END CLASS " << ast->name; )
    }
}


void gen_expression(AST_PTR ast) {
    DEBUG( "TODO: GEN_EXPRESSION"; )
    for (auto m: ast->members_by_order) {
        generate_code(m);
    }
}


void gen_function(AST_PTR ast) {
    auto func = std::dynamic_pointer_cast<FunctionAST>(ast);
    DEBUG( "BEGIN " << (func->is_static ? "STATIC" : "") << " FUNCTION " << func->name; )
    for (auto p: func->parameters_by_order) {
        DEBUG( "ADD PARAMETER " << p; )
    }
    if (func->takes_varargs) {
        DEBUG( "ADD VARARGS"; )
    }
    for (auto m: func->members_by_order) {
        generate_code(m);
    }
    DEBUG( "END FUNCTION " << func->name << std::endl; )
}


void gen_alias(AST_PTR ast) {
    DEBUG( "TODO: GEN_ALIAS"; )
}


void gen_conditional(AST_PTR ast) {
    DEBUG( "TODO: GEN_CONDITIONAL"; )
}


void generate_code(AST_PTR ast) {
    // The action we take here depend on what type of AST we are dealing with.
    switch (ast->type) {
        case AST_SCOPE:         gen_scope(ast);        break;
        case AST_RAW:           gen_raw(ast);          break;

        // Structural
        case AST_LABEL:         gen_label(ast);        break;
        case AST_VARIABLE:      gen_variable(ast);     break;
        case AST_FUNCTION:      gen_function(ast);     break;
        case AST_ALIAS:         gen_alias(ast);        break;
        case AST_CONDITIONAL:   gen_conditional(ast);  break;

        // Expressions
        case AST_EXPRESSION:
        case AST_IDENTIFIER:
        case AST_BINARY:
        case AST_STRING:
        case AST_LONG:
        case AST_DOUBLE:
        case AST_NULL:
        case AST_REFERENCE:
        case AST_OPERATOR:      gen_expression(ast);   break;

        // Uh oh.
        default: throw domain_error("Unknown AST. This is probably a bug.");
    }
}
