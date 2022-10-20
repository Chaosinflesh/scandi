// Scandi: lexer.h
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#pragma once
#include <iostream>
#include <string>
#include <vector>


enum TokenType {
    TOK_SCOPE             =  48,  // Spaces at the start of a line.  Also line
                                  //   scope + 1 at the end of a line.
    TOK_COMMENT           =  50,  // ` to end of line

    TOK_LABEL_DECL        =  64,  // \.
    TOK_VARIABLE_DECL     =  65,  // $
    TOK_VARIABLE_STATIC   =  66,  // $$
    TOK_FUNCTION_DECL     =  67,  // @
    TOK_FUNCTION_STATIC   =  68,  // @@

    TOK_IDENTIFIER        =  80,  // All token/whitespace-delimited words
    TOK_ALIAS_BEGIN       =  81,  // {
    TOK_ALIAS_END         =  82,  // }

    TOK_HEXADECIMAL       =  97,  // # - a number in hexadecimal follows
    TOK_NUMBER_DOUBLE     =  98,  // All numbers containing ','
    TOK_NUMBER_LONG       =  99,  // All numbers that are NOT double
    TOK_STRING            = 100,  // Contiguous blocks delimited by ' or "

    TOK_NULL              = 112,  // ()
    TOK_VARARGS           = 113,  // [] in function declaration context
    TOK_CONTENTS          = 114,  // [] with context
    TOK_SELF_CONTENTS     = 115,  // [] without context
    TOK_COUNT             = 116,  // !  with context
    TOK_SELF_COUNT        = 117,  // !  without context
    TOK_DOT               = 118,  // .  with context
    TOK_SELF_DOT          = 119,  // .  without context
    TOK_REFERENCE_BEGIN   = 120,  // [  with context
    TOK_REFERENCE_END     = 121,  // ]
    TOK_SELF_REFERENCE    = 122,  // [  without context
    TOK_ADDRESS           = 123,  // _
   
    TOK_ASSIGNMENT        = 128,  // =
    TOK_NEGATE_BEGIN      = 129,  // (
    TOK_NEGATE_END        = 130,  // )
    
    TOK_ADD               = 144,  // +
    TOK_SUB               = 145,  // -
    TOK_MULTIPLY          = 146,  // *
    TOK_DIVIDE            = 147,  // /
    TOK_MODULUS           = 148,  // %

    TOK_COMPLEMENT        = 160,  // ~
    TOK_AND               = 161,  // &
    TOK_OR                = 162,  // |
    TOK_XOR               = 163,  // ^
    TOK_SHL               = 164,  // <-
    TOK_SHR               = 165,  // ->
    TOK_SSHR              = 166,  // >>
    
    TOK_EQ                = 176,  // ?
    TOK_LT                = 177,  // <
    TOK_LTE               = 178,  // ?<
    TOK_GT                = 179,  // >
    TOK_GTE               = 180,  // ?>
    TOK_ELSE              = 181,  // :
};


class Token {
    public:
        // Lexer information
        TokenType type;
        std::string s_val;
        int l_val;
        double d_val;
        
        // Debug information
        std::string filename;
        int line_no;
        size_t pos;
    
        Token(
            TokenType type,
            std::string s_val,
            long l_val,
            double d_val
        ) :
            type(type),
            s_val(s_val),
            l_val(l_val),
            d_val(d_val)
        {}

        // Value-less token.        
        Token(TokenType type) :
            type(type),
            s_val(std::string()),
            l_val(0L),
            d_val(0.0)
        {}

        // String-type token.
        Token(TokenType type, std::string s_val) :
            type(type),
            s_val(s_val),
            l_val(0L),
            d_val(0.0)
        {}

        // Long-type token
        Token(long l_val) :
            type(TOK_NUMBER_LONG),
            s_val(std::string()),
            l_val(l_val),
            d_val(0.0)
        {}

        // Double-type token
        Token(double d_val) :
            type(TOK_NUMBER_DOUBLE),
            s_val(std::string()),
            l_val(0L),
            d_val(d_val)
        {}

        // Inject debug information in.
        inline void set_debug_info(std::string filename, int line_no, size_t pos) {
            this->filename = filename;
            this->line_no = line_no;
            this->pos = pos;
        }
};
std::ostream& operator<<(std::ostream& outStream, const Token& token);


bool tokenize_stream(
    std::vector<Token>& tokens_out,
    std::istream& stream_in,
    const std::string filename
);
