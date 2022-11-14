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

#define OFFSET( level ) string(level < 0 ? 0 : level, ' ')

void gen_scope(SHARED(AST) ast) {
    DEBUG( OFFSET(ast->depth) << "NEW SCOPE " << ast->name; )
    // Add all the members recursively.
    for (auto c: ast->children) {
        generate_code(c);
    }
    DEBUG( OFFSET(ast->depth) << "EXIT SCOPE " << ast->name << endl; )
}


void gen_raw(SHARED(AST) ast) {
    DEBUG( OFFSET(ast->depth) << "<<< INJECT RAW LLVM IR >>>"; )
}


void gen_label(SHARED(AST) ast) {
    DEBUG( OFFSET(ast->depth) << "ADD LABEL " << ast->name; )
    for (auto c: ast->children) {
        generate_code(c);
    }
}


void gen_variable(SHARED(AST) ast) {
    bool is_class = !ast->children.empty();
    DEBUG( OFFSET(ast->depth) << "ADD " << (ast->get_property(AST::OPT_STATIC) ? "STATIC " : "") << (is_class ? "CLASS " : "VARIABLE ") << ast->name; )
    if (is_class) {
        for (auto c: ast->children) {
            generate_code(c);
        }
        DEBUG( OFFSET(ast->depth) << "END CLASS " << ast->name << endl; )
    }
}


void gen_operator(SHARED(AST) op) {
    string action = "TODO: OP " + op->name;
    if        (op->name == "=") { action = "  POP VALUE INTO NEXT POP (TODO: differentiate between var and function)";
    } else if (op->name == "+") { action = "  POP POP ADD PUSH";
    } else if (op->name == "-") { action = "  POP1 POP2 POP2-POP1 PUSH";
    } else if (op->name == "*") { action = "  POP POP MULTIPLY PUSH";
    } else if (op->name == "/") { action = "  POP1 POP2 POP2/POP1 PUSH";
    } else if (op->name == "%") { action = "  POP1 POP2 POP2%POP1 PUSH";
    } else if (op->name == "&") { action = "  POP POP AND PUSH";
    } else if (op->name == "|") { action = "  POP POP OR PUSH";
    } else if (op->name == "^") { action = "  POP POP XOR PUSH";
    } else if (op->name == "!") { action = "  POP COUNT PUSH";
    } else if (op->name == "?") { action = "  POP POP COMPARE_EQUAL PUSH";
    } else if (op->name == "<") { action = "  POP POP COMPARE_LESS_THAN PUSH";
    } else if (op->name == "?<"){ action = "  POP POP COMPARE_LESS_THAN_EQUAL PUSH";
    } else if (op->name == ">") { action = "  POP POP COMPARE_GREATER_THAN PUSH";
    } else if (op->name == "?>"){ action = "  POP POP COMPARE_GREATER_THAN_EQUAL PUSH";
    }
    DEBUG( OFFSET(op->depth) << action; )
}


void gen_expression(SHARED(AST) ast) {
    auto current = ast;
    // Skip place-holder.
    if (current->type == AST_EXPRESSION) {
        current = current->next;
    }
    DEBUG( OFFSET(current->depth) << " INIT EXPRESSION STACK"; )
    while (current) {
        switch (current->type) {
            case AST_EXPRESSION:    gen_expression(current->alt);                                       break;
            case AST_IDENTIFIER:
            case AST_BINARY:
            case AST_STRING:        DEBUG( OFFSET(current->depth) << "  PUSH " << current->name << " ONTO EXPRESSION STACK"; )     break;
            case AST_LONG:          DEBUG( OFFSET(current->depth) << "  PUSH " << current->numeric_value.l << " ONTO EXPRESSION STACK"; )     break;
            case AST_DOUBLE:        DEBUG( OFFSET(current->depth) << "  PUSH " << current->numeric_value.d << " ONTO EXPRESSION STACK"; )     break;
            case AST_NULL:          DEBUG( OFFSET(current->depth) << "  PUSH NULL ONTO EXPRESSION STACK"; )     break;
            case AST_REFERENCE:     DEBUG( OFFSET(current->depth) << "  TODO: REF " << current->name; )     break;
            case AST_OPERATOR:      gen_operator(current); break;
            default:
                DERR("Unknown EXPRESSION. This is probably a bug.");
        }
        current = current->next;
    }
    DEBUG( OFFSET(ast->depth) << " PUSH EXPRESSION STACK IF PARENT STACK"; )
}


void gen_function(SHARED(AST) ast) {
    DEBUG( OFFSET(ast->depth) << "ADD " << (ast->get_property(AST::OPT_STATIC) ? "STATIC " : "") << "FUNCTION " << ast->name; )
    // Parameters
    auto next = ast->next;
    while (next) {
        DEBUG( OFFSET(ast->depth) << "ADD PARAMETER " << next->name; )
        next = next->next;
    }
    if (ast->get_property(AST::OPT_HAS_VARARGS)) {
        DEBUG( OFFSET(ast->depth) << "ADD VARARGS"; )
    }
    for (auto c: ast->children) {
        generate_code(c);
    }
    DEBUG( OFFSET(ast->depth) << "END FUNCTION " << ast->name << endl; )
}


void gen_alias(SHARED(AST) ast) {
    DEBUG( OFFSET(ast->depth) << "ADD INLINE FUNCTION " << ast->name; )
    generate_code(ast->next);
    DEBUG( OFFSET(ast->depth) << "END INLINE FUNCTION " << ast->name; )
}


void gen_conditional(SHARED(AST) ast) {
    DEBUG( OFFSET(ast->depth) << "ADD CONDITIONAL " << ast->name; )
    DEBUG( OFFSET(ast->depth) << "IF..."; )
    generate_code(ast->next);
    DEBUG( OFFSET(ast->depth) << "THEN..."; )
    for (auto c: ast->children) {
        generate_code(ast->next);
    }
    if (ast->alt) {
        DEBUG( OFFSET(ast->depth) << "ELSE..."; )
        // Skip the auto-label
        for (auto e: ast->alt->children) {
            generate_code(e);
        }
        //generate_code(ast->alt);
    }
    DEBUG( OFFSET(ast->depth) << "END CONDITIONAL " << ast->name; )
}


void generate_code(SHARED(AST) ast) {
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
        default: DERR("Unknown AST. This is probably a bug.");
    }
}
