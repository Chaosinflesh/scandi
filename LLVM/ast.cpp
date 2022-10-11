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
    std::string depth(alias.depth, ' ');
    os << depth << "Alias[" << alias.name << "] -> ";
    if (alias.target) {
        os << *alias.target;
    } else {
        os << "<>";
    }
    os << std::endl;
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


/*
std::ostream& operator<<(std::ostream& os, const ReferenceAST& ref) {
    os << "Reference -> " << ref.target ? ref.target.get() : "<>";
    return os;
}
*/
