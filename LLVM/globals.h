// Scandi: globals.h
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#pragma once


extern bool debug_set;


#define DEBUG( ... ) if (debug_set) { std::cerr << std::endl << __VA_ARGS__ }
