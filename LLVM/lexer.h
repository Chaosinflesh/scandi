#pragma once
#include <iostream>
#include <string>
#include <vector>


enum TokenType {
	TOK_SCOPE                       =  -1,
	TOK_IDENTIFIER_VARIABLE         =  -2,
	TOK_IDENTIFIER_FUNCTION         =  -3,
	TOK_IDENTIFIER_LABEL            =  -4,
	TOK_IDENTIFIER_ALIAS            =  -5,
	TOK_DECLARATION_VARIABLE        =  -6,
	TOK_DECLARATION_VARIABLE_STATIC =  -7,
	TOK_DECLARATION_FUNCTION        =  -8,
	TOK_DECLARATION_FUNCTION_STATIC =  -9,
	TOK_DECLARATION_LABEL           = -10,
	TOK_ADDRESS                     = -11,
	TOK_WITH_START                  = -12,
	TOK_WITH_END                    = -13,
	TOK_NUMBER_LONG                 = -14,
	TOK_NUMBER_DOUBLE               = -15,
	TOK_STRING                      = -16,
	TOK_BINARY_BLOB                 = -17,
	TOK_NULL                        = -18,
	TOK_VARARGS                     = -19,
	TOK_ASSIGNMENT                  = -20,
	TOK_EQ                          = -21,
	TOK_LT                          = -22,
	TOK_LTE                         = -23,
	TOK_GT                          = -24,
	TOK_GTE                         = -25,
	TOK_ELSE                        = -26,
	TOK_COMPLEMENT                  = -27,
	TOK_NEGATE_START                = -28,
	TOK_NEGATE_END                  = -29,
	TOK_AND                         = -30,
	TOK_OR                          = -31,
	TOK_XOR                         = -32,
	TOK_ADD                         = -33,
	TOK_SUBTRACT                    = -34,
	TOK_MULTIPLY                    = -35,
	TOK_DIVIDE                      = -36,
	TOK_MODULUS                     = -37,
	TOK_SHL                         = -38,
	TOK_SHR                         = -39,
	TOK_SSHR                        = -40,
	TOK_DOT                         = -41,
	TOK_REF_START                   = -42,
	TOK_REF_END                     = -43,
	TOK_COUNT                       = -44,
	TOK_CONTENTS                    = -45,
	TOK_NUMBER_HEXADECIMAL          = -46,
	TOK_EOL                         = -98,
	TOK_EOF                         = -99
};


class Token {
	public:
		TokenType type;
		std::string sVal;
		int lVal;
		double dVal;
	
		Token(TokenType type, std::string sVal, long lVal, double dVal) {
			this->type = type;
			this->sVal = sVal;
			this->lVal = lVal;
			this->dVal = dVal;
		}
		
		Token(TokenType type) {
			this->type = type;
			this->sVal = std::string();
			this->lVal = 0L;
			this->dVal = 0.0D;
		}
		
		Token(TokenType type, std::string sVal) {
			this->type = type;
			this->sVal = sVal;
			this->lVal = 0L;
			this->dVal = 0.0D;
		}
		
		Token(long lVal) {
			this->type = TOK_NUMBER_LONG;
			this->sVal = std::string();
			this->lVal = lVal;
			this->dVal = 0.0D;
		}
		
		Token(double dVal) {
			this->type = TOK_NUMBER_DOUBLE;
			this->sVal = std::string();
			this->lVal = 0L;
			this->dVal = dVal;
		}
};


void tokenizeLine(std::vector<Token>& tokens, const std::string line);


std::ostream& operator<<(std::ostream& o, const Token& t);
