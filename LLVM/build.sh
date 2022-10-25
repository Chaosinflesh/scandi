#!/bin/sh
#
# DEBUG DEBUG_AST DEBUG_PARSER DEBUG_SEMANTICS
#

clear
clang++ -DDEBUG -DDEBUG_AST --std=c++11 -Wall -g scandi.cpp lexer.cpp ast.cpp parser.cpp semantics.cpp -o scandi

# Test
./scandi $@
