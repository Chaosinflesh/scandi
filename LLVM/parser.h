// Scandi: parser.h
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#pragma once
#include <vector>
#include "ast.h"
#include "lexer.h"


AST_PTR parse_to_ast(std::vector<Token>, AST_PTR);
