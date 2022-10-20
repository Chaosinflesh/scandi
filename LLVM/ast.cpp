// Scandi: ast.cpp
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#include <iostream>
#include <string>
#include <typeinfo>
#include "ast.h"


/******************************************************************************
 *                               ScopeAST                                     *
 ******************************************************************************/

std::string ScopeAST::get_name() const {
    return name;
}

ASTType ScopeAST::get_type() const {
    return type;
}


int ScopeAST::get_depth() const {
    return depth;
}


bool ScopeAST::get_is_static() const {
    return is_static;
}


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
    std::shared_ptr<ScopeAST> member
) {
    if (member->depth > parent->depth) {
        if (parent->has_member_visible(member->name)) {
            throw std::domain_error("Member " + member->name + " already defined for this scope!");
        }
        parent->members_by_order.push_back(member);
        parent->members_by_name.emplace(member->name, member);
        member->parent_scope = parent;
        return member;
    } else {
        return ScopeAST::add_member(parent->parent_scope, member);
    }
}


std::ostream& operator<<(std::ostream& os, const ScopeAST& ast) {
    switch(ast.get_type()) {
        case AST_LABEL: os << dynamic_cast<const LabelAST&>(ast); break;
        case AST_VARIABLE: os << dynamic_cast<const VariableAST&>(ast); break;
        case AST_FUNCTION: os << dynamic_cast<const FunctionAST&>(ast); break;
        default:
            // Don't print the global scope.
            if (ast.depth >= 0) {
                os << std::endl << std::string(ast.depth, ' ') << ast.name << "[ScopeAST]";
            }
            for (auto member: ast.members_by_order) {
                os << *member;
            }
            break;
    }
    return os;
}


/******************************************************************************
 *                              LabelAST                                      *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const LabelAST& label) {
    os << " " << label.name << "[LabelAST]";
    return os;
}


/******************************************************************************
 *                             VariableAST                                    *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const VariableAST& variable) {
    os << " " << variable.name << "[VariableAST]" << (variable.get_is_static() ? "STATIC" : "");
    return os;
}


/******************************************************************************
 *                             FunctionAST                                    *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const FunctionAST& function) {
    os << " " << function.name << "[FunctionAST]" << (function.get_is_static() ? "STATIC" : "");
    return os;
}
