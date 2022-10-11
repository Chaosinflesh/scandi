#!/bin/sh
clear
clang++ --std=c++11 -Wall -g *.cpp -o scandi

# Test
cat $1 | ./scandi 
