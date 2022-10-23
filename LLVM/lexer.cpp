// Scandi: lexer.cpp
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include "lexer.h"

#define LEX_SPACE             ' '
#define LEX_COMMENT           '`'

#define LEX_LABEL_DECL        '\\'
#define LEX_VARIABLE_DECL     '$'
#define LEX_VARIABLE_STATIC   "$$"
#define LEX_FUNCTION_DECL     '@'
#define LEX_FUNCTION_STATIC   "@@"

#define LEX_ALIAS_BEGIN       '{'
#define LEX_ALIAS_END         '}'

#define LEX_ADDRESS           "_"
#define LEX_HEXADECIMAL       '#'
#define LEX_DECIMAL_POINT     ','
#define LEX_STRING_1          '\''
#define LEX_STRING_2          '"'

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


// Note that context-aware symbols & digraphs are handle in their own routine.

// Context-agnostic digraphs
std::map<std::string, TokenType> digraph_map {
    {LEX_VARIABLE_STATIC, TOK_VARIABLE_STATIC},
    {LEX_FUNCTION_STATIC, TOK_FUNCTION_STATIC},
    {LEX_ADDRESS,         TOK_ADDRESS},
    {LEX_NULL,            TOK_NULL},
    {LEX_SHL,             TOK_SHL},
    {LEX_SHR,             TOK_SHR},
    {LEX_SSHR,            TOK_SSHR},
    {LEX_LTE,             TOK_LTE},
    {LEX_GTE,             TOK_GTE}
};

// Context-agnostic symbols
std::map<char, TokenType> symbol_map {
    {LEX_LABEL_DECL,      TOK_LABEL_DECL},
    {LEX_VARIABLE_DECL,   TOK_VARIABLE_DECL},
    {LEX_FUNCTION_DECL,   TOK_FUNCTION_DECL},
    {LEX_ALIAS_BEGIN,     TOK_ALIAS_BEGIN},
    {LEX_ALIAS_END,       TOK_ALIAS_END},
    {LEX_HEXADECIMAL,     TOK_HEXADECIMAL},
    {LEX_REFERENCE_END,   TOK_REFERENCE_END},
    {LEX_ASSIGNMENT,      TOK_ASSIGNMENT},
    {LEX_NEGATE_BEGIN,    TOK_NEGATE_BEGIN},
    {LEX_NEGATE_END,      TOK_NEGATE_END},
    {LEX_ADD,             TOK_ADD},
    {LEX_SUB,             TOK_SUB},
    {LEX_MULTIPLY,        TOK_MULTIPLY},
    {LEX_DIVIDE,          TOK_DIVIDE},
    {LEX_MODULUS,         TOK_MODULUS},
    {LEX_COMPLEMENT,      TOK_COMPLEMENT},
    {LEX_AND,             TOK_AND},
    {LEX_OR,              TOK_OR},
    {LEX_XOR,             TOK_XOR},
    {LEX_EQ,              TOK_EQ},
    {LEX_LT,              TOK_LT},
    {LEX_GT,              TOK_GT},
    {LEX_ELSE,            TOK_ELSE}
};


// Ensure consistency of scope.
inline size_t get_file_scope_offset(size_t pos) {
    return pos + 1;
}


// Identifiers are contiguous blocks of alphanumeric characters starting with a
// letter.
size_t get_identifier(
    std::vector<Token>& tokens_out,
    const std::string line_in,
    size_t pos,
    const std::string filename,
    const int line_no
) {
    std::string identifier;
    auto init_pos = pos;
    while (pos < line_in.size() && isalnum(line_in[pos])) {
        identifier += line_in[pos];
        pos++;
    }
    auto token = Token(TOK_IDENTIFIER, identifier);
    token.set_debug_info(filename, line_no, init_pos);
    tokens_out.push_back(token);
    return pos;
}


// Numbers are digits that may contain a single decimal point.
size_t get_number(
    std::vector<Token>& tokens_out,
    const std::string line_in,
    size_t pos,
    const std::string filename,
    const int line_no
) {
    std::string number;
    bool decimal_seen = false;
    size_t init_pos = pos;

    // Build the number up.
    while (pos < line_in.size() && (isdigit(line_in[pos]) || line_in[pos] == LEX_DECIMAL_POINT)) {
        if (line_in[pos] == LEX_DECIMAL_POINT) {
            // Oops. malformed number!
            if (decimal_seen) {
                throw std::domain_error("Extraneous decimal point");
            } else {
                decimal_seen = true;
            }
            number += ".";
        } else {
            number += line_in[pos];
        }
        pos++;
    }

    // Create the correct type of token.
    if (decimal_seen) {
        auto token = Token(std::stod(number));
        token.set_debug_info(filename, line_no, init_pos);
        tokens_out.push_back(token);
    } else {
        auto token = Token(std::stol(number));
        token.set_debug_info(filename, line_no, init_pos);
        tokens_out.push_back(token);
    }
    return pos;
}


size_t get_string(
    std::vector<Token>& tokens_out,
    const std::string line_in,
    size_t pos,
    const std::string filename,
    const int line_no
) {
    std::string str;
    char delimiter = line_in[pos];
    bool in_string = true;
    size_t init_pos = pos;
    pos++;
    
    while (pos < line_in.size() && in_string) {
        if (line_in[pos] == delimiter) {
            // Need to check next char is not a continuation.
            pos++;
            if (pos < line_in.size() && (line_in[pos] == LEX_STRING_1 || line_in[pos] == LEX_STRING_2)) {
                delimiter = line_in[pos];
                pos++;
            } else {
                in_string = false;
            }
        } else {
            str += line_in[pos];
            pos++;
        }
    }
    
    auto token = Token(TOK_STRING, str);
    token.set_debug_info(filename, line_no, init_pos);
    tokens_out.push_back(token);
    return pos;
}


size_t get_symbol(
    std::vector<Token>& tokens_out,
    const std::string line_in,
    size_t pos,
    const std::string filename,
    const int line_no
) {
    std::string local = line_in.substr(pos);

    // Context-dependent lexes first. Note that the context-dependent lexes will
    // also trigger at the start of an alias.
    bool in_context = (
        pos > 0
     && line_in[pos - 1] != LEX_SPACE
     && line_in[pos - 1] != LEX_ALIAS_BEGIN
    );
    if (line_in[pos] == LEX_COUNT) {
        auto token = Token(in_context ? TOK_COUNT : TOK_SELF_COUNT);
        token.set_debug_info(filename, line_no, pos);
        tokens_out.push_back(token);
        return ++pos;

    } else if (line_in[pos] == LEX_DOT) {
        auto token = Token(in_context ? TOK_DOT : TOK_SELF_DOT);
        token.set_debug_info(filename, line_no, pos);
        tokens_out.push_back(token);
        return ++pos;

    } else if (local.rfind(LEX_VARARGS_CONTENTS, 0) == 0) {
        // Note that self-contents will be corrected to vararg during semantic
        // analysis if appropriate.
        auto token = Token(in_context ? TOK_CONTENTS : TOK_SELF_CONTENTS);
        token.set_debug_info(filename, line_no, pos);
        tokens_out.push_back(token);
        return pos + 2;

    } else if (line_in[pos] == LEX_REFERENCE_BEGIN) {
        auto token = Token(in_context ? TOK_REFERENCE_BEGIN : TOK_SELF_REFERENCE);
        token.set_debug_info(filename, line_no, pos);
        tokens_out.push_back(token);
        return ++pos;

    }

    // Digraphs second
    for (auto& d : digraph_map) {
        if (local.rfind(d.first, 0) == 0) {
            auto token = Token(d.second);
            token.set_debug_info(filename, line_no, pos);
            tokens_out.push_back(token);
            return pos + 2;
        }
    }
    
    // Other symbols
    for (auto& s: symbol_map) {
        if (line_in[pos] == s.first) {
            auto token = Token(s.second);
            token.set_debug_info(filename, line_no, pos);
            tokens_out.push_back(token);
            return ++pos;
        }
    }
    
    // If we get here, there was a problem!
    throw std::domain_error("Unknown symbol");
}


void tokenize_line(
    std::vector<Token>& tokens_out,
    const std::string line_in,
    const std::string filename,
    const int line_no
) {
    if (!line_in.empty()) {
        size_t pos = 0;

        // Check scope. File scope is 0, global is -1.
        while (pos < line_in.size() && line_in[pos] == LEX_SPACE) {
            pos++;
        }
        int init_pos = get_file_scope_offset(pos);
        auto token = Token(TOK_SCOPE, std::string(), init_pos, 0.0);
        token.set_debug_info(filename, line_no, pos);
        tokens_out.push_back(token);
        
        try {
            while (pos < line_in.size()) {
                // Check for comment
                if (line_in[pos] == LEX_COMMENT) {
                    auto token = Token(TOK_COMMENT, line_in.substr(pos + 1));
                    token.set_debug_info(filename, line_no, pos);
                    tokens_out.push_back(token);
                    break;
                }

                // Check for identifiers
                if (isalpha(line_in[pos])) {
                    pos = get_identifier(tokens_out, line_in, pos, filename, line_no);

                // Check for numbers
                } else if (isdigit(line_in[pos]) || line_in[pos] == LEX_DECIMAL_POINT) {
                    pos = get_number(tokens_out, line_in, pos, filename, line_no);

                // Check for strings
                } else if (line_in[pos] == LEX_STRING_1 || line_in[pos] == LEX_STRING_2) {
                    pos = get_string(tokens_out, line_in, pos, filename, line_no);

                // Skips spaces
                } else if (line_in[pos] == LEX_SPACE) {
                    pos++;

                // Check for implemented symbols.                    
                } else {
                    pos = get_symbol(tokens_out, line_in, pos, filename, line_no);
                    
                }
            }
        } catch (const std::domain_error& de) {
            throw std::domain_error(std::to_string(pos) + ": " + de.what());
        }
    }
}


bool tokenize_stream(
    std::vector<Token>& tokens_out,
    std::istream& stream_in,
    const std::string filename
) {
    bool success = true;
    int line_no = 1;

    // Enter file scope
    auto token = Token(TOK_SCOPE, filename);
    token.set_debug_info(filename, 0, 0);
    tokens_out.push_back(token);
    
    for (std::string line; std::getline(stream_in, line); ) {
        try {
            tokenize_line(tokens_out, line, filename, line_no);
        } catch (std::domain_error& de) {
            std::cerr << "LEXER: " << filename << "@" << line_no << ": " << de.what() << std::endl;
            success = false;
        }
        line_no++;
    }
    
    return success;
}


std::ostream& operator<<(std::ostream& o, const Token& t) {
    o << std::string(t.pos, ' ');
    o << "[" << t.type << ",\"" << t.s_val << "\"," << t.l_val << "," << t.d_val << "]";
#ifdef DEBUG
    o << "(" << t.filename << ":" << t.line_no << "@" << t.pos << ")";
#endif
    return o;
}


bool is_token_conditional(TokenType tt) {
    return tt >= 176 && tt <= 180;
}
