#!/bin/sh
#

clear
clang++ --std=c++11 -Wall -g scandi.cpp lexer.cpp ast.cpp parser.cpp semantics.cpp codegen.cpp -o scandi

# Test
./scandi $@
