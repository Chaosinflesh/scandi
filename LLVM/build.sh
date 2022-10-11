#!/bin/sh
clear
clang++ --std=c++11 -Wall -g *.cpp -o scandi

# Test
cat ../Examples/AdventOfCode/2015/3.scandi | ./scandi 
