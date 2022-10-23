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
    std::shared_ptr<ScopeAST> member,
    bool is_declaration
) {
    if (member->depth > parent->depth) {
        if (is_declaration) {
            if (parent->has_member_visible(member->name)) {
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
    switch(ast.get_type()) {
        case AST_LABEL: os << dynamic_cast<const LabelAST&>(ast); break;
        case AST_VARIABLE: os << dynamic_cast<const VariableAST&>(ast); break;
        case AST_FUNCTION: os << dynamic_cast<const FunctionAST&>(ast); break;
        case AST_EXPRESSION: os << dynamic_cast<const ExpressionAST&>(ast); break;
        case AST_ALIAS: os << dynamic_cast<const AliasAST&>(ast); break;
        case AST_CONDITIONAL: os << dynamic_cast<const ConditionalAST&>(ast); break;
        default:
            // Don't print the global scope.
            if (ast.depth >= 0) {
                os << std::endl << std::string(ast.depth, ' ') << ast.name << "[ScopeAST]";
            }
            break;
    }
    for (auto member: ast.members_by_order) {
        os << *member;
    }
    return os;
}


/******************************************************************************
 *                              LabelAST                                      *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const LabelAST& label) {
    os << std::endl << std::string(label.depth, ' ') << label.name << "[LabelAST]";
    return os;
}


/******************************************************************************
 *                             VariableAST                                    *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const VariableAST& variable) {
    os << std::endl << std::string(variable.depth, ' ') << variable.name << "[VariableAST]" << (variable.get_is_static() ? "STATIC" : "");
    return os;
}


/******************************************************************************
 *                             FunctionAST                                    *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const FunctionAST& function) {
    os << std::endl << std::string(function.depth, ' ') << function.name << "[FunctionAST]" << (function.get_is_static() ? "STATIC" : "");
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
    os << std::endl << std::string(ex.depth, ' ') << "[ExpressionAST]";
    // TODO: Display stack here.
    return os;
}


/******************************************************************************
 *                                AliasAST                                    *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const AliasAST& alias) {
    os << std::endl << std::string(alias.depth, ' ') << alias.name << "[AliasAST] ->";
    os << *alias.expression;
    return os;
}


void AliasAST::add_expression(std::shared_ptr<ExpressionAST> ex) {
    this->expression = ex;
}


std::shared_ptr<ExpressionAST> AliasAST::get_expression() {
    return this->expression;
}


/******************************************************************************
 *                            ConditionalAST                                  *
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const ConditionalAST& cond) {
    os << std::endl << std::string(cond.depth, ' ') << "[ConditionalAST] ->";
    os << *cond.expression;
    if (cond.when_true) {
        os << *cond.when_true;
    }
    if (cond.when_false) {
        os << *cond.when_false;
    }
    return os;
}


void ConditionalAST::set_target(bool when_true, std::shared_ptr<ScopeAST> scope) {
    if (when_true) {
        this->when_true = scope;
    } else {
        this->when_false = scope;
    }
}

std::shared_ptr<ScopeAST> ConditionalAST::get_target(bool when_true) {
    if (when_true) {
        return this->when_true;
    } else {
        return this->when_false;
    }
}

