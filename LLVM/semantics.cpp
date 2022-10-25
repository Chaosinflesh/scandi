// Scandi: semantics.cpp
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
#include "semantics.h"

/*
 *  Semantics analysis has a number of tasks to do:
 *
 *  1. It needs to expand auto-assignments ( e.g. n 1 + ).
 *  2. It needs to replace/map aliases as possible.
 *  3. It needs to map (where possible) identifiers to declarations.
 *  4. It needs to link conditionals with their auto_else if present.
 *  5. It needs to check static access where possible.
 *
 *  Note that it doesn't check stack validity.
 */

AST_PTR analyse_semantics(AST_PTR ast) {
#ifdef DEBUG_SEMANTICS
    std::cerr << std::endl << "ANALYSING SEMANTICS: TODO" << std::endl;
#endif
    return ast;
}
