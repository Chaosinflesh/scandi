// Scandi: lexer.cpp
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "lexer.h"


// Context-agnostic symbols
std::vector<int> operators {
    LEX_REFERENCE_END,
    LEX_ASSIGNMENT,
    LEX_NEGATE_BEGIN,
    LEX_NEGATE_END,
    LEX_ADDRESS,
    LEX_ADD,
    LEX_SUB,
    LEX_MULTIPLY,
    LEX_DIVIDE,
    LEX_MODULUS,
    LEX_COMPLEMENT,
    LEX_AND,
    LEX_OR,
    LEX_XOR,
    LEX_EQ,
    LEX_LT,
    LEX_GT,
    LEX_ELSE
};


const bool Token::is_assignment() const {
    return this->type == TOK_OPERATOR && this->s_val == std::string(1, LEX_ASSIGNMENT);
}


const bool Token::is_conditional() const {
    return this->type == TOK_OPERATOR && (
        this->s_val == std::string(1, LEX_EQ)
     || this->s_val == std::string(1, LEX_GT)
     || this->s_val == LEX_GTE
     || this->s_val == std::string(1, LEX_LT)
     || this->s_val == LEX_LTE
    );
}

#define FN( NAME )      size_t NAME (std::vector<Token>& tokens_out, const std::string line_in, const std::string filename, const int line_no, size_t pos)
#define INIT_POS        auto init_pos = pos
#define DEBUG_POS       filename, line_no, pos
#define DEBUG_INIT_POS  filename, line_no, init_pos
#define TOK_ADD( ... )  tokens_out.push_back(Token( __VA_ARGS__ ))


// Identifiers are contiguous blocks of alphanumeric characters starting with a
// letter.
FN( get_identifier ) {
    INIT_POS;
    std::string identifier;

    while (pos < line_in.size() && isalnum(line_in[pos])) {
        identifier += line_in[pos];
        pos++;
    }

    if (!tokens_out.empty() && tokens_out.back().s_val.empty() && (
        tokens_out.back().type == TOK_FUNCTION
     || tokens_out.back().type == TOK_VARIABLE
     || tokens_out.back().type == TOK_LABEL
    )) {
        tokens_out.back().s_val = identifier;
    } else {
        TOK_ADD(TOK_IDENTIFIER, identifier, 0L, 0.0, false, false, DEBUG_INIT_POS);
    }
    return pos;
}


// Numbers are digits that may contain a single decimal point.
FN( get_number ) {
    INIT_POS;
    std::string number;
    bool decimal_seen = false;

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
        TOK_ADD(TOK_VALUE, LEX_DECIMAL_POINT, 0L, std::stod(number), false, false, DEBUG_INIT_POS);
    } else {
        TOK_ADD(TOK_VALUE, "", std::stol(number), 0.0, false, false, DEBUG_INIT_POS);
    }
    return pos;
}


FN( get_string ) {
    INIT_POS;
    std::string str;
    char delimiter = line_in[pos];
    bool in_string = true;
    pos++;
    
    while (pos < line_in.size() && in_string) {
        if (line_in[pos] == delimiter) {
            // Need to check next char is not a continuation.
            pos++;
            if (pos < line_in.size() && (line_in[pos] == LEX_QUOTE || line_in[pos] == LEX_APOSTROPHE)) {
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
    TOK_ADD(TOK_STRING, str, 0L, 0.0, false, false, DEBUG_INIT_POS);
    return pos;
}


FN( get_hex ) {
    INIT_POS;
    pos++;

    if (pos == line_in.size()) {
        throw std::domain_error("Reached end of line on hexadecimal");
    }
    if (line_in[pos] == LEX_QUOTE || line_in[pos] == LEX_APOSTROPHE) {
        // Binary blob.
        pos = get_string(tokens_out, line_in, DEBUG_POS);
        tokens_out.back().type = TOK_BINARY;
    } else {
        // Hex number.
        std::string num = "";
        while (pos < line_in.size() && std::isxdigit(line_in[pos])) {
            num += line_in[pos];
            pos++;
        }
        TOK_ADD(TOK_VALUE, "", std::stol(num, nullptr, 16), 0.0, false, false, DEBUG_INIT_POS);
        
    }
    return pos;
}


#define CONSIDERING_STRING(TEST)        if (local.rfind(TEST, 0) == 0) {
#define ALSO_CONSIDERING_STRING(TEST)   } else if (local.rfind(TEST, 0) == 0) {
#define CONSIDERING_CHAR(TEST)          if (local[0] == TEST) {
#define ALSO_CONSIDERING_CHAR(TEST)     } else if (local[0] == TEST) {
#define STOP_CONSIDERING                }
#define RETURN_POS_INC                  return ++pos
#define RETURN_POS_INC2                 return pos + 2

FN( get_symbol ) {
    std::string local = line_in.substr(pos);
    bool in_context = (pos > 0 && line_in[pos - 1] != LEX_SPACE && line_in[pos - 1] != LEX_ALIAS_BEGIN);

    // Context-dependent lexes first. Note that the context-dependent lexes will also trigger at the start of an alias.
    CONSIDERING_CHAR(LEX_COUNT)                     TOK_ADD(TOK_OPERATOR, LEX_COUNT, 0L, 0.0, false, !in_context, DEBUG_POS);               RETURN_POS_INC;
    ALSO_CONSIDERING_CHAR(LEX_DOT)                  TOK_ADD(TOK_OPERATOR, LEX_DOT, 0L, 0.0, false, !in_context, DEBUG_POS);                 RETURN_POS_INC;
    // Note that self-contents will be corrected to vararg during semantic analysis if appropriate.
    ALSO_CONSIDERING_STRING(LEX_VARARGS_CONTENTS)   TOK_ADD(TOK_OPERATOR, LEX_VARARGS_CONTENTS, 0L, 0.0, false, !in_context, DEBUG_POS);    RETURN_POS_INC2;
    ALSO_CONSIDERING_CHAR(LEX_REFERENCE_BEGIN)      TOK_ADD(TOK_OPERATOR, LEX_REFERENCE_BEGIN, 0L, 0.0, false, !in_context, DEBUG_POS);     RETURN_POS_INC;

    // Context-less lexes: digraphs.
    ALSO_CONSIDERING_STRING(LEX_VARIABLE_STATIC)    TOK_ADD(TOK_VARIABLE, "", 0L, 0.0, true, false, DEBUG_POS);                             RETURN_POS_INC2;
    ALSO_CONSIDERING_STRING(LEX_FUNCTION_STATIC)    TOK_ADD(TOK_FUNCTION, "", 0L, 0.0, true, false, DEBUG_POS);                             RETURN_POS_INC2;
    ALSO_CONSIDERING_STRING(LEX_NULL)               TOK_ADD(TOK_VALUE, LEX_NULL, 0L, 0.0, true, false, DEBUG_POS);                          RETURN_POS_INC2;
    ALSO_CONSIDERING_STRING(LEX_SHL)                TOK_ADD(TOK_OPERATOR, LEX_SHL, 0L, 0.0, true, false, DEBUG_POS);                        RETURN_POS_INC2;
    ALSO_CONSIDERING_STRING(LEX_SHR)                TOK_ADD(TOK_OPERATOR, LEX_SHR, 0L, 0.0, true, false, DEBUG_POS);                        RETURN_POS_INC2;
    ALSO_CONSIDERING_STRING(LEX_SSHR)               TOK_ADD(TOK_OPERATOR, LEX_SSHR, 0L, 0.0, true, false, DEBUG_POS);                       RETURN_POS_INC2;
    ALSO_CONSIDERING_STRING(LEX_GTE)                TOK_ADD(TOK_OPERATOR, LEX_GTE, 0L, 0.0, true, false, DEBUG_POS);                        RETURN_POS_INC2;
    ALSO_CONSIDERING_STRING(LEX_LTE)                TOK_ADD(TOK_OPERATOR, LEX_LTE, 0L, 0.0, true, false, DEBUG_POS);                        RETURN_POS_INC2;

    // Structural single lexes
    ALSO_CONSIDERING_CHAR(LEX_LABEL_DECL)           TOK_ADD(TOK_LABEL, "", 0L, 0.0, false, false, DEBUG_POS);                               RETURN_POS_INC;
    ALSO_CONSIDERING_CHAR(LEX_VARIABLE_DECL)        TOK_ADD(TOK_VARIABLE, "", 0L, 0.0, false, false, DEBUG_POS);                            RETURN_POS_INC;
    ALSO_CONSIDERING_CHAR(LEX_FUNCTION_DECL)        TOK_ADD(TOK_FUNCTION, "", 0L, 0.0, false, false, DEBUG_POS);                            RETURN_POS_INC;
    ALSO_CONSIDERING_CHAR(LEX_ALIAS_BEGIN)          TOK_ADD(TOK_ALIAS_BEGIN, "", 0L, 0.0, false, false, DEBUG_POS);                         RETURN_POS_INC;
    ALSO_CONSIDERING_CHAR(LEX_ALIAS_END)            TOK_ADD(TOK_ALIAS_END, "", 0L, 0.0, false, false, DEBUG_POS);                           RETURN_POS_INC;
    STOP_CONSIDERING

    // Single lexe operators
    for (auto op: operators) {
        CONSIDERING_CHAR(op)                        TOK_ADD(TOK_OPERATOR, op, 0L, 0.0, false, false, DEBUG_POS);                            RETURN_POS_INC;
        STOP_CONSIDERING
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
    size_t pos = 0;

    // Check scope. File scope is 0, global is -1.
    while (pos < line_in.size() && line_in[pos] == LEX_SPACE) {
        pos++;
    }

    // Don't duplicate scopes (caused by comment-only lines).
    if (tokens_out.back().type != TOK_SCOPE || tokens_out.back().l_val != pos + 1) {
        TOK_ADD(TOK_SCOPE, std::string(), pos + 1, 0.0, false, false, DEBUG_POS);
    }
    
    try {
        while (pos < line_in.size()) {
            // Check for comment
            if (line_in[pos] == LEX_COMMENT) {
                return;
            }

            // Check for hexadecimal
            if (line_in[pos] == LEX_HEXADECIMAL) {
                pos = get_hex(tokens_out, line_in, DEBUG_POS);
                
            // Check for identifiers
            } else if (isalpha(line_in[pos])) {
                pos = get_identifier(tokens_out, line_in, DEBUG_POS);

            // Check for numbers
            } else if (isdigit(line_in[pos]) || line_in[pos] == LEX_DECIMAL_POINT) {
                pos = get_number(tokens_out, line_in, DEBUG_POS);

            // Check for strings
            } else if (line_in[pos] == LEX_QUOTE || line_in[pos] == LEX_APOSTROPHE) {
                pos = get_string(tokens_out, line_in, DEBUG_POS);

            // Skips spaces
            } else if (line_in[pos] == LEX_SPACE) {
                pos++;

            // Check for implemented symbols.                    
            } else {
                pos = get_symbol(tokens_out, line_in, DEBUG_POS);
                
            }
        }
    } catch (const std::domain_error& de) {
        throw std::domain_error(std::to_string(pos) + ": " + de.what());
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
    TOK_ADD(TOK_SCOPE, filename, 0L, 0.0, true, false, filename, 0, 0);
    for (std::string line; std::getline(stream_in, line); ) {
        try {
            if (!line.empty()) {
                tokenize_line(tokens_out, line, filename, line_no);
            }
        } catch (std::domain_error& de) {
            std::cerr << "LEXER: " << filename << "@" << line_no << ": " << de.what() << std::endl;
            success = false;
        }
        line_no++;
    }
    
    return success;
}


std::ostream& operator<<(std::ostream& o, const Token& t) {
    if (t.type == TOK_SCOPE) {
        o << std::endl << std::string(t.pos, ' ');
    } else {
        o << std::string(1, t.type) << (t.targets_self ? "!" : "") << (t.is_static ? "+" : "") << ".";
        switch (t.type) {
            case TOK_LABEL:
            case TOK_VARIABLE:
            case TOK_FUNCTION:
            case TOK_IDENTIFIER:
            case TOK_BINARY:
            case TOK_STRING:
            case TOK_OPERATOR:
                o << t.s_val;
                break;

            default:
                if (t.s_val == std::string(1, LEX_DECIMAL_POINT)) {
                    o << t.d_val;
                } else if (t.s_val.empty()) {
                    o << t.l_val;
                } else {
                    o << t.s_val;
                }
                break;
        }
        o << " ";
    }
    return o;
}
