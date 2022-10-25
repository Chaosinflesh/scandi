// Scandi: globals.h
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#pragma once


extern bool DEBUG_FLAG;


#define DEBUG( ... ) if (DEBUG_FLAG) { std::cerr << std::endl << __VA_ARGS__ }


