// Scandi: ast.h
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#pragma once
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "lexer.h"


enum ASTType {
    AST_SCOPE,

    // Structures
    AST_LABEL,
    AST_VARIABLE,
    AST_FUNCTION,
    AST_ALIAS,

    // Types
    AST_LONG,
    AST_DOUBLE,
    AST_STRING,
    AST_BLOB,
    AST_NULL,
    AST_VARARGS,

    // Operators
    AST_JUMP,               // An unconditional jump.
    AST_ADDRESS,
    AST_DOT,
    AST_CONTENTS,
    AST_COUNT,
    AST_REFERENCE,          // This is a name for the label, variable and function LUT.

    AST_ASSIGNMENT,
    AST_UNARY_OP,           
    AST_BINARY_OP,          // All the arithmetic and logical operators.
    AST_CONDITIONAL_OP      // Contains 2 jump values, which is taken depends on previous stack value.
};


/******************************************************************************
 *                             ScopeAST                                       *
 ******************************************************************************/
class ScopeAST {

    private:
        std::string name;                                                       // Useful for debugging.
        ASTType type = AST_SCOPE;                                               // Used to determine which << operator to use.
        int depth;                                                              // This is useful for tree building.
        bool is_static;                                                         // This is useful for semantic checking.
        std::shared_ptr<ScopeAST> parent_scope;                                 // It is necessary to know parent for semantic checking.
        std::vector<std::shared_ptr<ScopeAST>> members_by_order;                // Execution happens in order here.
        std::map<std::string, std::shared_ptr<ScopeAST>> members_by_name;       // Used for building the reference connections.
        
    public:
        ScopeAST(
            std::string name,
            ASTType type,
            int depth,
            bool is_static
        ) :
            name(name),
            type(type),
            depth(depth),
            is_static(is_static)
        {}

        std::string get_name() const;
        ASTType get_type() const;
        int get_depth() const;
        bool get_is_static() const;
        bool has_direct_member(const std::string&) const;
        bool has_member_visible(const std::string&) const;
        virtual ~ScopeAST() {}

        // Adds the member. If the member does not belong in this scope (as
        // determined by depth, passes it up the chain. Returns the scope it
        // belongs to.
        static std::shared_ptr<ScopeAST> add_member(
            std::shared_ptr<ScopeAST>,
            std::shared_ptr<ScopeAST>
        );
        
        friend std::ostream& operator<<(std::ostream&, const ScopeAST&);
};
std::ostream& operator<<(std::ostream&, const ScopeAST&);
