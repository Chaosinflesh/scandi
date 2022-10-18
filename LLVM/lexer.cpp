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

#define LEX_ADDRESS           "__"
#define LEX_HEXADECIMAL       '#'
#define LEX_DECIMAL_POINT     ','
#define LEX_STRING_1          '\''
#define LEX_STRING_2          '"'
#define LEX_BINARY_BLOB       '_'

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
std::map<std::string, TokenType> digraphMap {
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
std::map<char, TokenType> symbolMap {
    {LEX_LABEL_DECL,      TOK_LABEL_DECL},
    {LEX_VARIABLE_DECL,   TOK_VARIABLE_DECL},
    {LEX_FUNCTION_DECL,   TOK_FUNCTION_DECL},
    {LEX_ALIAS_BEGIN,     TOK_ALIAS_BEGIN},
    {LEX_ALIAS_END,       TOK_ALIAS_END},
    {LEX_HEXADECIMAL,     TOK_HEXADECIMAL},
    {LEX_BINARY_BLOB,     TOK_BINARY_BLOB},
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
inline size_t getFileScopeOffset(size_t pos) {
    return pos + 1;
}


// Identifiers are contiguous blocks of alphanumeric characters starting with a
// letter.
size_t getIdentifier(
    std::vector<Token>& tokensOut,
    const std::string lineIn,
    size_t pos,
    const std::string filename,
    const int lineNo
) {
    std::string identifier;
    auto init_pos = pos;
    while (pos < lineIn.size() && isalnum(lineIn[pos])) {
        identifier += lineIn[pos];
        pos++;
    }
    auto token = Token(TOK_IDENTIFIER, identifier);
    token.setDebugInfo(filename, lineNo, init_pos);
    tokensOut.push_back(token);
    return pos;
}


// Numbers are digits that may contain a single decimal point.
size_t getNumber(
    std::vector<Token>& tokensOut,
    const std::string lineIn,
    size_t pos,
    const std::string filename,
    const int lineNo
) {
    std::string number;
    bool decimalSeen = false;
    size_t init_pos = pos;

    // Build the number up.
    while (pos < lineIn.size() && (isdigit(lineIn[pos]) || lineIn[pos] == LEX_DECIMAL_POINT)) {
        if (lineIn[pos] == LEX_DECIMAL_POINT) {
            // Oops. malformed number!
            if (decimalSeen) {
                throw std::domain_error("Extraneous decimal point");
            } else {
                decimalSeen = true;
            }
            number += ".";
        } else {
            number += lineIn[pos];
        }
        pos++;
    }

    // Create the correct type of token.
    if (decimalSeen) {
        auto token = Token(std::stod(number));
        token.setDebugInfo(filename, lineNo, init_pos);
        tokensOut.push_back(token);
    } else {
        auto token = Token(std::stol(number));
        token.setDebugInfo(filename, lineNo, init_pos);
        tokensOut.push_back(token);
    }
    return pos;
}


size_t getString(
    std::vector<Token>& tokensOut,
    const std::string lineIn,
    size_t pos,
    const std::string filename,
    const int lineNo
) {
    std::string str;
    char delimiter = lineIn[pos];
    bool inString = true;
    size_t init_pos = pos;
    pos++;
    
    while (pos < lineIn.size() && inString) {
        if (lineIn[pos] == delimiter) {
            // Need to check next char is not a continuation.
            pos++;
            if (pos < lineIn.size() && (lineIn[pos] == LEX_STRING_1 || lineIn[pos] == LEX_STRING_2)) {
                delimiter = lineIn[pos];
                pos++;
            } else {
                inString = false;
            }
        } else {
            str += lineIn[pos];
            pos++;
        }
    }
    
    auto token = Token(TOK_STRING, str);
    token.setDebugInfo(filename, lineNo, init_pos);
    tokensOut.push_back(token);
    return pos;
}


size_t getSymbol(
    std::vector<Token>& tokensOut,
    const std::string lineIn,
    size_t pos,
    const std::string filename,
    const int lineNo
) {
    std::string local = lineIn.substr(pos);

    // Context-dependent lexes first. Note that the context-dependent lexes will
    // also trigger at the start of an alias.
    bool inContext = (
        pos > 0
     && lineIn[pos - 1] != LEX_SPACE
     && lineIn[pos - 1] != LEX_ALIAS_BEGIN
    );
    if (lineIn[pos] == LEX_COUNT) {
        auto token = Token(inContext ? TOK_COUNT : TOK_SELF_COUNT);
        token.setDebugInfo(filename, lineNo, pos);
        tokensOut.push_back(token);
        return ++pos;

    } else if (lineIn[pos] == LEX_DOT) {
        auto token = Token(inContext ? TOK_DOT : TOK_SELF_DOT);
        token.setDebugInfo(filename, lineNo, pos);
        tokensOut.push_back(token);
        return ++pos;

    } else if (local.rfind(LEX_VARARGS_CONTENTS, 0) == 0) {
        // Note that self-contents will be corrected to vararg during semantic
        // analysis if appropriate.
        auto token = Token(inContext ? TOK_CONTENTS : TOK_SELF_CONTENTS);
        token.setDebugInfo(filename, lineNo, pos);
        tokensOut.push_back(token);
        return pos + 2;

    } else if (lineIn[pos] == LEX_REFERENCE_BEGIN) {
        auto token = Token(inContext ? TOK_REFERENCE_BEGIN : TOK_SELF_REFERENCE);
        token.setDebugInfo(filename, lineNo, pos);
        tokensOut.push_back(token);
        return ++pos;

    }

    // Digraphs second
    for (auto& d : digraphMap) {
        if (local.rfind(d.first, 0) == 0) {
            auto token = Token(d.second);
            token.setDebugInfo(filename, lineNo, pos);
            tokensOut.push_back(token);
            return pos + 2;
        }
    }
    
    // Other symbols
    for (auto& s: symbolMap) {
        if (lineIn[pos] == s.first) {
            auto token = Token(s.second);
            token.setDebugInfo(filename, lineNo, pos);
            tokensOut.push_back(token);
            return ++pos;
        }
    }
    
    // If we get here, there was a problem!
    throw std::domain_error("Unknown symbol");
}


void tokenizeLine(
    std::vector<Token>& tokensOut,
    const std::string lineIn,
    const std::string filename,
    const int lineNo
) {
    if (!lineIn.empty()) {
        size_t pos = 0;

        // Check scope. File scope is 0, global is -1.
        while (pos < lineIn.size() && lineIn[pos] == LEX_SPACE) {
            pos++;
        }
        int init_pos = getFileScopeOffset(pos);
        auto token = Token(TOK_SCOPE_DOWN, std::string(), init_pos, 0.0);
        token.setDebugInfo(filename, lineNo, pos);
        tokensOut.push_back(token);
        
        try {
            while (pos < lineIn.size()) {
                // Check for comment
                if (lineIn[pos] == LEX_COMMENT) {
                    auto token = Token(TOK_COMMENT, lineIn.substr(pos + 1));
                    token.setDebugInfo(filename, lineNo, pos);
                    tokensOut.push_back(token);
                    break;
                }

                // Check for identifiers
                if (isalpha(lineIn[pos])) {
                    pos = getIdentifier(tokensOut, lineIn, pos, filename, lineNo);

                // Check for numbers
                } else if (isdigit(lineIn[pos]) || lineIn[pos] == LEX_DECIMAL_POINT) {
                    pos = getNumber(tokensOut, lineIn, pos, filename, lineNo);

                // Check for strings
                } else if (lineIn[pos] == LEX_STRING_1 || lineIn[pos] == LEX_STRING_2) {
                    pos = getString(tokensOut, lineIn, pos, filename, lineNo);

                // Skips spaces
                } else if (lineIn[pos] == LEX_SPACE) {
                    pos++;

                // Check for implemented symbols.                    
                } else {
                    pos = getSymbol(tokensOut, lineIn, pos, filename, lineNo);
                    
                }
            }
        } catch (const std::domain_error& de) {
            throw std::domain_error(std::to_string(pos) + ": " + de.what());
        }

        // Calculate the next immediate scope (may not be used, Parser will sort
        // it out.
        token = Token(TOK_SCOPE_DOWN, std::string(), init_pos + 1, 0.0);
        token.setDebugInfo(filename, lineNo, lineIn.size());
        tokensOut.push_back(token);
    }
}


bool tokenizeStream(
    std::vector<Token>& tokensOut,
    std::istream& streamIn,
    const std::string filename
) {
    bool success = true;
    int lineNo = 1;

    // Enter file scope
    auto token = Token(TOK_SCOPE_DOWN, filename);
    token.setDebugInfo(filename, 0, 0);
    tokensOut.push_back(token);
    
    for (std::string line; std::getline(streamIn, line); ) {
        try {
            tokenizeLine(tokensOut, line, filename, lineNo);
        } catch (std::domain_error& de) {
            std::cerr << "LEXER: " << filename << "@" << lineNo << ": " << de.what() << std::endl;
            success = false;
        }
        lineNo++;
    }

    // Exit file scope
    token = Token(TOK_SCOPE_UP, std::string(), -1, 0.0);
    token.setDebugInfo(filename, lineNo, 0);
    tokensOut.push_back(token);
    
    return success;
}


std::ostream& operator<<(std::ostream& o, const Token& t) {
    o << std::string(t.pos, ' ');
    o << "[" << t.type << ",\"" << t.sVal << "\"," << t.lVal << "," << t.dVal << "]";
#ifdef DEBUG
    o << "(" << t.filename << ":" << t.lineNo << "@" << t.pos << ")";
#endif
    return o;
}
