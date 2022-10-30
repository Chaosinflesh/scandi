// Scandi: parser.h
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#pragma once
#include <vector>
#include "ast.h"
#include "globals.h"
#include "lexer.h"


using std::vector;


SHARED(AST) parse_to_ast(vector<Token>, SHARED(AST));
