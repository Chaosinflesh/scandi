// Scandi: parser.h
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#pragma once
#include <memory>
#include <vector>
#include "ast.h"
#include "lexer.h"


bool parse_to_ast(std::vector<Token>, std::shared_ptr<ScopeAST>);
