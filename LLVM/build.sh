#!/bin/sh
clear
clang++ -DDEBUG --std=c++11 -Wall -g scandi.cpp lexer.cpp ast.cpp parser.cpp -o scandi

# Test
cat $1 | ./scandi 
