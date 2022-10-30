// Scandi: globals.h
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#pragma once
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>


using std::cerr;
using std::cout;
using std::domain_error;
using std::endl;
using std::map;
using std::ostream;
using std::string;
using std::vector;


extern bool debug_set;


#define CHAR_STR( ch )      string(1, ch )

#define DEBUG( ... )        if (debug_set) { cout << endl << __VA_ARGS__ }
#define DERR( ... )         throw domain_error( __VA_ARGS__ )

#define SHARE( type, ... )  std::make_shared< type >( type ( __VA_ARGS__ ))
#define SHARED( type )      std::shared_ptr< type >
