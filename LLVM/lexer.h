// Scandi: lexer.h
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#pragma once
#include <iostream>
#include <string>
#include <vector>


#define LEX_SPACE             ' '
#define LEX_COMMENT           '`'

#define LEX_LABEL_DECL        '\\'
#define LEX_VARIABLE_DECL     '$'
#define LEX_VARIABLE_STATIC   "$$"
#define LEX_FUNCTION_DECL     '@'
#define LEX_FUNCTION_STATIC   "@@"

#define LEX_RAW_BEGIN         "{{"
#define LEX_RAW_END           "}}"
#define LEX_ALIAS_BEGIN       '{'
#define LEX_ALIAS_END         '}'

#define LEX_ADDRESS           '_'
#define LEX_HEXADECIMAL       '#'
#define LEX_DECIMAL_POINT     ','
#define LEX_QUOTE             '\''
#define LEX_APOSTROPHE        '"'

#define LEX_NULL              "()"
#define LEX_VARARGS_CONTENTS  "[]"
#define LEX_COUNT             '!'
#define LEX_DOT               '.'
#define LEX_REFERENCE_BEGIN   '['
#define LEX_REFERENCE_END     ']'
   
#define LEX_ASSIGNMENT        '='
#define LEX_NEGATE_BEGIN      '('
#define LEX_NEGATE_END        ')'
    
#define LEX_ADD               '+'
#define LEX_SUB               '-'
#define LEX_MULTIPLY          '*'
#define LEX_DIVIDE            '/'
#define LEX_MODULUS           '%'

#define LEX_COMPLEMENT        '~'
#define LEX_AND               '&'
#define LEX_OR                '|'
#define LEX_XOR               '^'
#define LEX_SHL               "<-"
#define LEX_SHR               "->"
#define LEX_SSHR              ">>"
    
#define LEX_EQ                '?'
#define LEX_LT                '<'
#define LEX_LTE               "?<"
#define LEX_GT                '>'
#define LEX_GTE               "?>"
#define LEX_ELSE              ':'


enum TokenType {
    TOK_SCOPE             = 'S',  // Spaces at the start of a line.
    TOK_LABEL             = 'L',  // \.
    TOK_VARIABLE          = 'V',  // $
    TOK_FUNCTION          = 'F',  // @
    TOK_ALIAS_BEGIN       = 'A',  // {
    TOK_ALIAS_END         = 'a',  // }
    TOK_IDENTIFIER        = 'I',  // All token/whitespace-delimited words
    TOK_VALUE             = 'v',  // All values use a shared type
    TOK_BINARY            = 'B',  //  .. except a binary array!
    TOK_STRING            = 's',  // All values use a shared type
    TOK_OPERATOR          = 'O',  // Operators are really functions
    TOK_RAW               = 'R'   // Raw code lines, passed into LLVM
};


class Token {
    public:
        // Lexer information
        TokenType type;
        std::string s_val;
        int l_val;
        double d_val;
        bool is_static;
        bool targets_self;
        
        // Debug information
        std::string filename;
        int line_no;
        size_t pos;
    
        Token(
            TokenType type,
            std::string s_val,
            long l_val,
            double d_val,
            bool is_static,
            bool targets_self,
            std::string filename,
            int line_no,
            size_t pos
        ) :
            type(type),
            s_val(s_val),
            l_val(l_val),
            d_val(d_val),
            is_static(is_static),
            targets_self(targets_self),
            filename(filename),
            line_no(line_no),
            pos(pos)
        {}


        Token(
            TokenType type,
            char s_val,
            long l_val,
            double d_val,
            bool is_static,
            bool targets_self,
            std::string filename,
            int line_no,
            size_t pos
        ) :
            type(type),
            s_val(std::string(1, s_val)),
            l_val(l_val),
            d_val(d_val),
            is_static(is_static),
            targets_self(targets_self),
            filename(filename),
            line_no(line_no),
            pos(pos)
        {}


        const bool is_assignment() const;
        const bool is_conditional() const;

};
std::ostream& operator<<(std::ostream& outStream, const Token& token);


bool tokenize_stream(
    std::vector<Token>& tokens_out,
    std::istream& stream_in,
    const std::string filename
);
