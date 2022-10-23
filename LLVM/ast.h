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

    // Structural
    AST_LABEL,
    AST_VARIABLE,
    AST_FUNCTION,
    AST_ALIAS,
    AST_EXPRESSION,
    AST_CONDITIONAL,

    // Types
    AST_IDENTIFIER,
    AST_LONG,
    AST_DOUBLE,
    AST_STRING,
    AST_BLOB,
    AST_NULL,
    AST_VARARGS,

    // Operators
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

    protected:
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
            int depth,
            bool is_static
        ) :
            name(name),
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
            std::shared_ptr<ScopeAST>,
            bool
        );

        friend std::ostream& operator<<(std::ostream&, const ScopeAST&);
};
std::ostream& operator<<(std::ostream&, const ScopeAST&);


/******************************************************************************
 *                              LabelAST                                      *
 ******************************************************************************/
class LabelAST : public ScopeAST {

    public:
        LabelAST(
            std::string name,
            int depth
        ) :
            ScopeAST(name, depth, false)
        {
            type = AST_LABEL;
        }

        friend std::ostream& operator<<(std::ostream&, const LabelAST&);
};
std::ostream& operator<<(std::ostream&, const LabelAST&);


/******************************************************************************
 *                             VariableAST                                    *
 ******************************************************************************/
class VariableAST : public ScopeAST {

    public:
        VariableAST(
            std::string name,
            int depth,
            bool is_static
        ) :
            ScopeAST(name, depth, is_static)
        {
            type = AST_VARIABLE;
        }

        friend std::ostream& operator<<(std::ostream&, const VariableAST&);
};
std::ostream& operator<<(std::ostream&, const VariableAST&);


/******************************************************************************
 *                             FunctionAST                                    *
 ******************************************************************************/
class FunctionAST : public ScopeAST {

    protected:
        bool takes_varargs;
        std::vector<std::string> parameters_by_order;
        std::map<std::string, std::shared_ptr<VariableAST>> parameters_by_name;

    public:
        FunctionAST(
            std::string name,
            int depth,
            bool is_static,
            bool takes_varargs
        ) :
            ScopeAST(name, depth, is_static),
            takes_varargs(takes_varargs)
        {
            type = AST_FUNCTION;
        }

        void add_parameter(std::string);

        friend std::ostream& operator<<(std::ostream&, const FunctionAST&);
};
std::ostream& operator<<(std::ostream&, const FunctionAST&);


/******************************************************************************
 *                             ExpressionAST                                  *
 ******************************************************************************/
class ExpressionAST : public ScopeAST {

    public:
        ExpressionAST(
            int depth
        ) :
            ScopeAST("", depth, false)
        {
            type = AST_EXPRESSION;
        }

        friend std::ostream& operator<<(std::ostream&, const ExpressionAST&);
};
std::ostream& operator<<(std::ostream&, const ExpressionAST&);


/******************************************************************************
 *                                AliasAST                                    *
 ******************************************************************************/
class AliasAST : public ScopeAST {

    protected:
        std::shared_ptr<ExpressionAST> expression;

    public:
        AliasAST(
            std::string name,
            int depth
        ) :
            ScopeAST(name, depth, true)
        {
            type = AST_ALIAS;
        }

        void add_expression(std::shared_ptr<ExpressionAST>);
        std::shared_ptr<ExpressionAST> get_expression();

        friend std::ostream& operator<<(std::ostream&, const AliasAST&);
};
std::ostream& operator<<(std::ostream&, const AliasAST&);


/******************************************************************************
 *                            ConditionalAST                                  *
 ******************************************************************************/
class ConditionalAST : public AliasAST {

    protected:
        std::shared_ptr<ScopeAST> when_true;
        std::shared_ptr<ScopeAST> when_false;

    public:
        ConditionalAST(
            int depth
        ) :
            AliasAST("", depth)
        {
            type = AST_CONDITIONAL;
        }

        void set_target(bool, std::shared_ptr<ScopeAST>);
        std::shared_ptr<ScopeAST> get_target(bool);

        friend std::ostream& operator<<(std::ostream&, const ConditionalAST&);
};
std::ostream& operator<<(std::ostream&, const ConditionalAST&);
