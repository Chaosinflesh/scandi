// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include "lexer.h"


#define LEX_SPACE                    ' '
#define LEX_COMMENT                  '`'
#define LEX_WITH_START               '{'
#define LEX_WITH_END                 '}'
#define LEX_LABEL                    '\\'
#define LEX_ADDRESS                  "__"
#define LEX_VARIABLE                 '$'
#define LEX_VARIABLE_STATIC          "$$"
#define LEX_FUNCTION                 '@'
#define LEX_FUNCTION_STATIC          "@@"
#define LEX_NUMBER_HEXADECIMAL       '#'
#define LEX_DECIMAL_POINT            ','
#define LEX_STRING_QUOTES            '"'
#define LEX_STRING_APOSTROPHE        '\''
#define LEX_BINARY                   '_'
#define LEX_NULL                     "()"
#define LEX_VARARGS                  "[]"
#define LEX_ASSIGNMENT               '='
#define LEX_COMPARISON_EQ            '?'
#define LEX_COMPARISON_LT            '<'
#define LEX_COMPARISON_LTE           "?<"
#define LEX_COMPARISON_GT            '>'
#define LEX_COMPARISON_GTE           "?>"
#define LEX_ELSE                     ':'
#define LEX_UNARY_COMPLEMENT         '~'
#define LEX_NEGATE_START             '('
#define LEX_NEGATE_END               ')'
#define LEX_BINARY_AND               '&'
#define LEX_BINARY_OR                '|'
#define LEX_BINARY_XOR               '^'
#define LEX_BINARY_ADD               '+'
#define LEX_BINARY_SUBTRACT          '-'
#define LEX_BINARY_MULTIPLY          '*'
#define LEX_BINARY_DIVIDE            '/'
#define LEX_BINARY_MODULUS           '%'
#define LEX_UNARY_SHIFT_LEFT         "<-"
#define LEX_UNARY_SHIFT_RIGHT        "->"
#define LEX_UNARY_SIGNED_SHIFT_RIGHT ">>"
#define LEX_DOT_OPERATOR             '.'
#define LEX_REFERENCE_START          '['
#define LEX_REFERENCE_END            ']'
#define LEX_COUNT                    '!'


std::map<std::string, TokenType> digraphMap {
    {LEX_ADDRESS,                  TOK_ADDRESS                    },
    {LEX_VARIABLE_STATIC,          TOK_DECLARATION_VARIABLE_STATIC},
    {LEX_FUNCTION_STATIC,          TOK_DECLARATION_FUNCTION_STATIC},
    {LEX_NULL,                     TOK_NULL                       },
    {LEX_VARARGS,                  TOK_VARARGS                    },
    {LEX_COMPARISON_LTE,           TOK_LTE                        },
    {LEX_COMPARISON_GTE,           TOK_GTE                        },
    {LEX_UNARY_SHIFT_LEFT,         TOK_SHL                        },
    {LEX_UNARY_SHIFT_RIGHT,        TOK_SHR                        },
    {LEX_UNARY_SIGNED_SHIFT_RIGHT, TOK_SSHR                       }
};


std::map<char, TokenType> symbolMap {
    {LEX_WITH_START,          TOK_WITH_START          },
    {LEX_WITH_END,            TOK_WITH_END            },
    {LEX_LABEL,               TOK_DECLARATION_LABEL   },
    {LEX_VARIABLE,            TOK_DECLARATION_VARIABLE},
    {LEX_FUNCTION,            TOK_DECLARATION_FUNCTION},
    {LEX_NUMBER_HEXADECIMAL,  TOK_NUMBER_HEXADECIMAL  },
    {LEX_BINARY,              TOK_BINARY_BLOB         },
    {LEX_ASSIGNMENT,          TOK_ASSIGNMENT          },
    {LEX_COMPARISON_EQ,       TOK_EQ                  },
    {LEX_COMPARISON_LT,       TOK_LT                  },
    {LEX_COMPARISON_GT,       TOK_GT                  },
    {LEX_ELSE,                TOK_ELSE                },
    {LEX_UNARY_COMPLEMENT,    TOK_COMPLEMENT          },
    {LEX_NEGATE_START,        TOK_NEGATE_START        },
    {LEX_NEGATE_END,          TOK_NEGATE_END          },
    {LEX_BINARY_AND,          TOK_AND                 },
    {LEX_BINARY_OR,           TOK_OR                  },
    {LEX_BINARY_XOR,          TOK_XOR                 },
    {LEX_BINARY_ADD,          TOK_ADD                 },
    {LEX_BINARY_SUBTRACT,     TOK_SUBTRACT            },
    {LEX_BINARY_MULTIPLY,     TOK_MULTIPLY            },
    {LEX_BINARY_DIVIDE,       TOK_DIVIDE              },
    {LEX_BINARY_MODULUS,      TOK_MODULUS             },
    {LEX_DOT_OPERATOR,        TOK_DOT                 },
    {LEX_REFERENCE_START,     TOK_REF_START           },
    {LEX_REFERENCE_END,       TOK_REF_END             },
    {LEX_COUNT,               TOK_COUNT               }
};


std::ostream& operator<<(std::ostream& o, const Token& t) {
    o << "[" << t.type << "]" << t.sVal << ": " << t.lVal << " " << t.dVal;
    return o;
}


size_t getIdentifier(std::vector<Token>& tokens, const std::string line, size_t pos) {
    std::string identifier;
    while (pos < line.size() && isalnum(line[pos])) {
        identifier += line[pos];
        pos++;
    }
    tokens.push_back(Token(TOK_IDENTIFIER_ALIAS, identifier));
    return pos;
}


size_t getNumber(std::vector<Token>& tokens, const std::string line, size_t pos) {
    std::string number;
    bool decimalSeen = false;
    while (pos < line.size() && (isdigit(line[pos]) || line[pos] == LEX_DECIMAL_POINT)) {
        if (line[pos] == LEX_DECIMAL_POINT) {
            // Oops. malformed number!
            if (decimalSeen) {
                throw std::domain_error("Extraneous decimal point");
            } else {
                decimalSeen = true;
            }
            number += ".";
        } else {
            number += line[pos];
        }
        pos++;
    }
    if (decimalSeen) {
        tokens.push_back(Token(std::stod(number)));
    } else {
        tokens.push_back(Token(std::stol(number)));
    }
    return pos;
}


size_t getString(std::vector<Token>& tokens, const std::string line, size_t pos) {
    std::string str;
    char delimiter = line[pos];
    bool inString = true;
    pos++;
    while (pos < line.size() && inString) {
        if (line[pos] == delimiter) {
            // Need to check next char is not a continuation.
            pos++;
            if (pos < line.size() && (line[pos] == LEX_STRING_QUOTES || line[pos] == LEX_STRING_APOSTROPHE)) {
                delimiter = line[pos];
                pos++;
            } else {
                inString = false;
            }
        } else {
            str += line[pos];
            pos++;
        }
    }
    tokens.push_back(Token(TOK_STRING, str));
    return pos;
}


size_t getSymbol(std::vector<Token>& tokens, const std::string line, size_t pos) {
    std::string local = line.substr(pos);
    
    // Digraphs first
    if (pos < line.size() - 1) {
        for (auto& d : digraphMap) {
            if (local.rfind(d.first, 0) == 0) {
                if (pos > 0 && d.second == TOK_VARARGS && line[pos - 1] != LEX_SPACE) {
                    tokens.push_back(Token(TOK_CONTENTS));
                } else {
                    tokens.push_back(Token(d.second));
                }
                return pos + 2;
            }
        }
    }
    
    // Other symbols
    for (auto& s: symbolMap) {
        if (line[pos] == s.first) {
            tokens.push_back(Token(s.second));
            return pos + 1;
        }
    }
    
    // If we get here, there was a problem!
    throw std::domain_error("Unknown symbol");
}


void tokenizeLine(std::vector<Token>& tokens, const std::string line) {
    if (!line.empty()) {
        size_t pos = 0;

        // Check scope. Scope within files starts at 1, as 0 is
        // considered global.
        while (pos < line.size() && line[pos] == LEX_SPACE) {
            pos++;
        }
        tokens.push_back(Token(TOK_SCOPE, std::string(), (int)pos + 1, 0.0));
        
        try {
            while (pos < line.size()) {
                // Check for comment
                if (line[pos] == LEX_COMMENT) {
                    return;
                }
                
                // Check for identifiers
                if (isalpha(line[pos])) {
                    pos = getIdentifier(tokens, line, pos);

                // Check for numbers
                } else if (isdigit(line[pos]) || line[pos] == LEX_DECIMAL_POINT) {
                    pos = getNumber(tokens, line, pos);
                
                // Check for strings
                } else if (line[pos] == LEX_STRING_QUOTES || line[pos] == LEX_STRING_APOSTROPHE) {
                    pos = getString(tokens, line, pos);

                // Check for symbols
                } else {
                    pos = getSymbol(tokens, line, pos);
                }
                
                // Skip spaces
                while (pos < line.size() && line[pos] == LEX_SPACE) {
                    pos++;
                }
            }
        } catch (const std::domain_error& de) {
            throw std::domain_error(std::to_string(pos) + ": " + de.what());
        }
    }
}


bool tokenizeStream(std::vector<Token>& tokens, std::istream& in, const std::string name) {
    bool success = true;
    int lineNo = 1;
    for (std::string line; std::getline(in, line); ) {
        try {
            tokenizeLine(tokens, line);
        } catch (std::domain_error& de) {
            std::cerr << "LEXER: " << name << "@" << lineNo << ": " << de.what() << std::endl;
            success = false;
        }
        lineNo++;
    }
    return success;
}
