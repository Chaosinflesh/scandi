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
    AST_BINARY,
    AST_STRING,
    AST_LONG,
    AST_DOUBLE,
    AST_NULL,

    // Operators
    AST_REFERENCE,
    AST_OPERATOR
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
        std::shared_ptr<ScopeAST> parent_scope = nullptr;                       // It is necessary to know parent for semantic checking.
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

    protected:
        std::shared_ptr<ExpressionAST> next = nullptr;

    public:
        ExpressionAST(
            int depth
        ) :
            ScopeAST("", depth, false)
        {
            type = AST_EXPRESSION;
        }

        void set_next(std::shared_ptr<ExpressionAST>);
        const std::shared_ptr<ExpressionAST> get_next() const;

        friend std::ostream& operator<<(std::ostream&, const ExpressionAST&);
};
std::ostream& operator<<(std::ostream&, const ExpressionAST&);


/******************************************************************************
 *                                AliasAST                                    *
 ******************************************************************************/
class AliasAST : public ScopeAST {

    protected:
        std::shared_ptr<ExpressionAST> expression = nullptr;

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
        std::string condition;
        std::shared_ptr<ScopeAST> when_true = nullptr;
        std::shared_ptr<ScopeAST> when_false = nullptr;

    public:
        ConditionalAST(
            std::string condition,
            int depth
        ) :
            AliasAST("", depth),
            condition(condition)
        {
            type = AST_CONDITIONAL;
        }

        const std::string get_condition() const;
        void set_target(bool, std::shared_ptr<ScopeAST>);
        std::shared_ptr<ScopeAST> get_target(bool);

        friend std::ostream& operator<<(std::ostream&, const ConditionalAST&);
};
std::ostream& operator<<(std::ostream&, const ConditionalAST&);


/******************************************************************************
 *                             IdentifierAST                                  *
 ******************************************************************************/
class IdentifierAST : public ExpressionAST {

    protected:
        // Ignoring for the moment some targets will be calculated.
        std::shared_ptr<ScopeAST> target = nullptr;

    public:
        IdentifierAST(
            std::string name,
            int depth
        ) :
            ExpressionAST(depth)
        {
            type = AST_IDENTIFIER;
            this->name = name;
        }

        void set_target(std::shared_ptr<ScopeAST>);
        const std::shared_ptr<ScopeAST> get_target() const;

        friend std::ostream& operator<<(std::ostream&, const IdentifierAST&);
};
std::ostream& operator<<(std::ostream&, const IdentifierAST&);


/******************************************************************************
 *                                StringAST                                   *
 ******************************************************************************/
class StringAST : public ExpressionAST {

    protected:
        std::string value;
        
    public:
        StringAST(
            std::string value,
            int depth
        ) :
            ExpressionAST(depth),
            value(value)
        {
            type = AST_STRING;
        }

        friend std::ostream& operator<<(std::ostream&, const StringAST&);
};
std::ostream& operator<<(std::ostream&, const StringAST&);


/******************************************************************************
 *                                BinaryAST                                   *
 ******************************************************************************/
class BinaryAST : public ExpressionAST {

    protected:
        std::vector<char> value;
        
    public:
        BinaryAST(
            std::string value,
            int depth
        ) :
            ExpressionAST(depth)
        {
            type = AST_BINARY;
            // TODO: convert value to value!
        }

        friend std::ostream& operator<<(std::ostream&, const BinaryAST&);
};
std::ostream& operator<<(std::ostream&, const BinaryAST&);


/******************************************************************************
 *                                 LongAST                                    *
 ******************************************************************************/
class LongAST : public ExpressionAST {

    protected:
        long value;
        
    public:
        LongAST(
            long value,
            int depth
        ) :
            ExpressionAST(depth),
            value(value)
        {
            type = AST_LONG;
        }

        friend std::ostream& operator<<(std::ostream&, const LongAST&);
};
std::ostream& operator<<(std::ostream&, const LongAST&);


/******************************************************************************
 *                                DoubleAST                                   *
 ******************************************************************************/
class DoubleAST : public ExpressionAST {

    protected:
        double value;
        
    public:
        DoubleAST(
            double value,
            int depth
        ) :
            ExpressionAST(depth),
            value(value)
        {
            type = AST_DOUBLE;
        }

        friend std::ostream& operator<<(std::ostream&, const DoubleAST&);
};
std::ostream& operator<<(std::ostream&, const DoubleAST&);


/******************************************************************************
 *                                 NullAST                                    *
 ******************************************************************************/
class NullAST : public ExpressionAST {

    public:
        NullAST(
            int depth
        ) :
            ExpressionAST(depth)
        {
            type = AST_NULL;
        }

        friend std::ostream& operator<<(std::ostream&, const NullAST&);
};
std::ostream& operator<<(std::ostream&, const NullAST&);


/******************************************************************************
 *                              OperatorAST                                   *
 ******************************************************************************/
class OperatorAST : public ExpressionAST {

    protected:
        std::string op;
        bool is_self;

    public:
        OperatorAST(
            std::string op,
            bool is_self,
            int depth
        ) :
            ExpressionAST(depth),
            op(op),
            is_self(is_self)
        {
            type = AST_OPERATOR;
        }

        const std::string get_op() const;
        const bool get_is_self() const;
};
std::ostream& operator<<(std::ostream&, const OperatorAST&);


/******************************************************************************
 *                              ReferenceAST                                  *
 ******************************************************************************/
class ReferenceAST : public OperatorAST {

    protected:
        // Ignoring for the moment some targets will be calculated.
        std::shared_ptr<ExpressionAST> expression = nullptr;

    public:
        ReferenceAST(
            bool is_self,
            int depth
        ) :
            OperatorAST("", is_self, depth)
        {
            type = AST_REFERENCE;
        }

        void add_expression(std::shared_ptr<ExpressionAST>);
        std::shared_ptr<ExpressionAST> get_expression();

        friend std::ostream& operator<<(std::ostream&, const ReferenceAST&);
};
std::ostream& operator<<(std::ostream&, const ReferenceAST&);

