// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0
#include <iostream>
#include <string>
#include <typeinfo>
#include "ast.h"


std::ostream& operator<<(std::ostream& os, const CitizenAST& citizen) {
    switch (citizen.printId) {
        case CITIZEN_DECLARED:
            os << dynamic_cast<const DeclaredCitizenAST&>(citizen);
            break;
            
        case CITIZEN_ALIAS:
            os << dynamic_cast<const AliasAST&>(citizen);
            break;
            
        case CITIZEN_FUNCTION:
            os << dynamic_cast<const FunctionAST&>(citizen);
            break;
            
        case CITIZEN_EXPRESSION:
            os << dynamic_cast<const ExpressionAST&>(citizen);
            break;
        
        case CITIZEN_OPERATOR:
            os << dynamic_cast<const OperatorAST&>(citizen);
            break;
        
        case CITIZEN_STRING:
            os << dynamic_cast<const StringAST&>(citizen);
            break;
        
        case CITIZEN_INTEGER:
            os << dynamic_cast<const IntegerAST&>(citizen);
            break;

        case CITIZEN_FLOAT:
            os << dynamic_cast<const FloatAST&>(citizen);
            break;

        case CITIZEN_COMPARATOR:
            os << dynamic_cast<const ComparatorAST&>(citizen);
            break;

        case CITIZEN_NULL:
            os << dynamic_cast<const NullAST&>(citizen);
            break;

        default:
            // Don't print global info.
            if (citizen.depth >= 0) {
                std::string depth(citizen.depth, ' ');
                os << depth << "Citizen:Unidentified" << std::endl;
            }
            for (auto member: citizen.members) {
                os << *member;
            }
            break;
    }
    return os;
}


std::ostream& operator<<(std::ostream& os, const DeclaredCitizenAST& citizen) {
    std::string depth(citizen.depth, ' ');
    std::string type;
    switch (citizen.type) {
        case TOK_DECLARATION_LABEL:           type = "DECLARATION_LABEL";           break;
        case TOK_DECLARATION_VARIABLE:        type = "DECLARATION_VARIABLE";        break;
    }
    os << depth << "Citizen[" << type << "]: " << citizen.name;
    if (citizen.isStatic) {
        os << " +STATIC";
    }
    os << std::endl;
    for (auto member: citizen.members) {
        os << *member;
    }
    return os;
}


std::ostream& operator<<(std::ostream& os, const AliasAST& alias) {
    os << alias.name << "->";
    if (alias.target) {
        os << *alias.target;
    } else {
        os << "<>";
    }
    // Make nice at file-level.
    if (alias.depth == 1) {
        os << std::endl;
    }
    return os;
}


std::ostream& operator<<(std::ostream& os, const FunctionAST& function) {
    std::string depth(function.depth, ' ');
    os << depth << "Function[" << function.name << "] -> ";
    if (function.isStatic) {
        os << " +STATIC";
    }
    if (function.takesVarargs) {
        os << " +VARARGS";
    }
    for (auto arg: function.arguments) {
        os << " " << dynamic_cast<DeclaredCitizenAST&>(*arg).name;
    }
    os << std::endl;
    for (auto member: function.members) {
        os << *member;
    }
    return os;
}


std::ostream& operator<<(std::ostream& os, const ExpressionAST& expression) {
    std::string depth(expression.depth, ' ');
    os << depth << "Expression:";
    for (auto s : expression.stack) {
        os << "  " << *s << "  ";
    }
    os << std::endl;
    for (auto member : expression.members) {
        os << *member;
    }
    return os;
}


std::ostream& operator<<(std::ostream& os, const GotoAST& go) {
    os << " " << go.name << "->";
    if (go.target) {
        os << *go.target;
    } else {
        os << "<>";
    }
    return os;
}


std::ostream& operator<<(std::ostream& os, const OperatorAST& op) {
    std::string opCode;
    switch (op.type) {
        case TOK_DOT: opCode = "."; break;
        case TOK_REF_START: opCode = "["; break;
        case TOK_REF_END: opCode = "]"; break;
        case TOK_ADD: opCode = "+"; break;
        case TOK_MULTIPLY: opCode = "*"; break;
        case TOK_SUBTRACT: opCode = "-"; break;
        case TOK_DIVIDE: opCode = "/"; break;
        case TOK_COMPLEMENT: opCode = "~"; break;
        case TOK_MODULUS: opCode = "%"; break;
        case TOK_AND: opCode = "&"; break;
        case TOK_OR: opCode = "|"; break;
        case TOK_XOR: opCode = "^"; break;
        case TOK_SHL: opCode = "<-"; break;
        case TOK_SHR: opCode = "->"; break;
        case TOK_SSHR: opCode = ">>"; break;
        case TOK_ASSIGNMENT: opCode = "="; break;
        case TOK_COUNT: opCode = "!"; break;
        case TOK_CONTENTS: opCode = "[]"; break;
        default:
            throw std::range_error("Invalid operator selected. This is a bug!");
    }
    os << opCode;
    return os;
}


std::ostream& operator<<(std::ostream& os, const ComparatorAST& op) {
    std::string opCode;
    switch (op.type) {
        case TOK_EQ: opCode = "?"; break;
        case TOK_LT: opCode = "<"; break;
        case TOK_LTE: opCode = "?<"; break;
        case TOK_GT: opCode = ">"; break;
        case TOK_GTE: opCode = "?>"; break;
        default:
            throw std::range_error("Invalid comparator selected. This is a bug!");
    }
    os << opCode;
    return os;
}


std::ostream& operator<<(std::ostream& os, const StringAST& str) {
    os << '"' << str.value << '"';
    return os;
}


std::ostream& operator<<(std::ostream& os, const IntegerAST& i) {
    os << i.value;
    return os;
}


std::ostream& operator<<(std::ostream& os, const FloatAST& f) {
    os << f.value;
    return os;
}


std::ostream& operator<<(std::ostream& os, const NullAST& f) {
    os << "()";
    return os;
}
