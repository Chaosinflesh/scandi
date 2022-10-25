// Scandi: ast.cpp
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#include <iostream>
#include <string>
#include <typeinfo>
#include "ast.h"
#include "globals.h"


/******************************************************************************
 *                               ScopeAST                                     *
 ******************************************************************************/
bool ScopeAST::has_direct_member(const std::string& name) const {
    return members_by_name.count(name) != 0;
}


bool ScopeAST::has_member_visible(const std::string& name) const {
    if (has_direct_member(name)) {
        return true;
    } else if (depth >= 0) {
        return parent_scope->has_member_visible(name);
    }
    return false;
}


std::shared_ptr<ScopeAST> ScopeAST::add_member(
    std::shared_ptr<ScopeAST> parent,
    std::shared_ptr<ScopeAST> member,
    bool is_declaration
) {
    if (member->depth > parent->depth) {
        if (is_declaration) {
            if (parent->has_member_visible(member->name)) {
                std::cerr << std::endl << parent->name;
                for (auto x: parent->members_by_name) {
                    std::cerr << " " << x.first;
                }
                throw std::domain_error("Member " + member->name + " already defined for this scope!");
            }
            parent->members_by_name.emplace(member->name, member);
        }
        parent->members_by_order.push_back(member);
        member->parent_scope = parent;
        return member;
    } else {
        return ScopeAST::add_member(parent->parent_scope, member, is_declaration);
    }
}


std::ostream& operator<<(std::ostream& os, const ScopeAST& ast) {
    if (ast.depth >= 0) {
        os << std::endl << std::string(ast.depth, ' ');
    }
    switch(ast.type) {
        case AST_RAW: os << dynamic_cast<const RawAST&>(ast); break;
        case AST_LABEL: os << dynamic_cast<const LabelAST&>(ast); break;
        case AST_VARIABLE: os << dynamic_cast<const VariableAST&>(ast); break;
        case AST_FUNCTION: os << dynamic_cast<const FunctionAST&>(ast); break;
        case AST_ALIAS: os << dynamic_cast<const AliasAST&>(ast); break;
        case AST_CONDITIONAL: os << dynamic_cast<const ConditionalAST&>(ast); return os;
        case AST_EXPRESSION: os << dynamic_cast<const ExpressionAST&>(ast); break;
        case AST_IDENTIFIER: os << dynamic_cast<const ExpressionAST&>(ast); break;
        case AST_STRING: os << dynamic_cast<const ExpressionAST&>(ast); break;
        case AST_LONG: os << dynamic_cast<const ExpressionAST&>(ast); break;
        case AST_DOUBLE: os << dynamic_cast<const ExpressionAST&>(ast); break;
        case AST_NULL: os << dynamic_cast<const ExpressionAST&>(ast); break;
        case AST_OPERATOR: os << dynamic_cast<const ExpressionAST&>(ast); break;
        case AST_REFERENCE: os << dynamic_cast<const ExpressionAST&>(ast); break;
        default:
            os << ast.name;
            break;
    }
    for (auto member: ast.members_by_order) {
        os << *member;
    }
    return os;
}


/******************************************************************************
 *                               RawAST                                       *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const RawAST& raw) {
    os << "{{" << raw.name << std::endl << raw.code << "}}";
    return os;
}


/******************************************************************************
 *                              LabelAST                                      *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const LabelAST& label) {
    os << "\\" << label.name;
    return os;
}


/******************************************************************************
 *                             VariableAST                                    *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const VariableAST& variable) {
    os << variable.name << (variable.is_static ? "+ " : " ");
    return os;
}


/******************************************************************************
 *                             FunctionAST                                    *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const FunctionAST& function) {
    os << "@" << function.name << (function.is_static ? "+ " : " ");
    for (auto parameter: function.parameters_by_order) {
        os << " " << parameter;
    }
    os << (function.takes_varargs ? " +" : "");
    return os;
}


void FunctionAST::add_parameter(std::string name) {
    parameters_by_order.push_back(name);
    parameters_by_name.emplace(name, std::make_shared<VariableAST>(VariableAST(
        name,
        this->depth,
        false
    )));
}

/******************************************************************************
 *                             ExpressionAST                                  *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const ExpressionAST& ex) {
    switch (ex.type) {
        case AST_IDENTIFIER: os << dynamic_cast<const IdentifierAST&>(ex); break;
        case AST_STRING:     os << dynamic_cast<const     StringAST&>(ex); break;
        case AST_LONG:       os << dynamic_cast<const       LongAST&>(ex); break;
        case AST_DOUBLE:     os << dynamic_cast<const     DoubleAST&>(ex); break;
        case AST_NULL:       os << dynamic_cast<const       NullAST&>(ex); break;
        case AST_OPERATOR:   os << dynamic_cast<const   OperatorAST&>(ex); break;
        case AST_REFERENCE:  os << dynamic_cast<const  ReferenceAST&>(ex); break;
        default:
            throw std::domain_error("This shouldn't have happened: " + std::to_string(ex.type));
        break;
    }
    if (ex.next) {
        os << *ex.next;
    }
    return os;
}


/******************************************************************************
 *                              ReferenceAST                                  *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const ReferenceAST& ref) {
    os << "[(" << (ref.is_self ? "! " : " ");
    if (ref.expression) {
        os << *ref.expression;
    }
    os << ")] ";
    return os;
}


/******************************************************************************
 *                                AliasAST                                    *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const AliasAST& alias) {
    os << alias.name << "-> ";
    if (alias.expression) {
        os << *alias.expression;
    }
    return os;
}


/******************************************************************************
 *                            ConditionalAST                                  *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const ConditionalAST& cond) {
    os << cond.condition << ") ";
    if (cond.expression) {
        os << *cond.expression;
    }
    os << std::endl << std::string(cond.depth, ' ') <<  "TRUE:";
    for (auto member : cond.members_by_order) {
        os << *member;
    }
    os << std::endl << std::string(cond.depth, ' ') << "FALSE:";
    if (cond.when_false) {
        os << *cond.when_false;
    }
    os << std::endl << std::string(cond.depth, ' ') << "AFTER:";
    return os;
}


/******************************************************************************
 *                             IdentifierAST                                  *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const IdentifierAST& id) {
    os << id.name << " ";
    if (id.target) {
        os << "->" << id.target->name;
    }
    return os;
}


/******************************************************************************
 *                                StringAST                                   *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const StringAST& s) {
    os << "\"" << s.value << "\" " ;
    return os;
}


/******************************************************************************
 *                                BinaryAST                                   *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const BinaryAST& s) {
    os << "b'" << std::hex;
    for (auto b: s.value) {
        os << (unsigned int)b;
    }
    return os << std::dec;
}


/******************************************************************************
 *                                 LongAST                                    *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const LongAST& l) {
    os <<  l.value << " ";
    return os;
}


/******************************************************************************
 *                                DoubleAST                                   *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const DoubleAST& d) {
    os <<  d.value << " ";
    return os;
}


/******************************************************************************
 *                                 NullAST                                    *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const NullAST& n) {
    os <<  "() ";
    return os;
}


/******************************************************************************
 *                              OperatorAST                                   *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const OperatorAST& o) {
    os << o.op << (o.is_static ? "+ " : " ");
    return os;
}

