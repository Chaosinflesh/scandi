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


#define AST_PTR std::shared_ptr<ScopeAST>


enum ASTType {
    AST_SCOPE = 1,
    AST_RAW = 2,

    // Structural
    AST_LABEL = 3,
    AST_VARIABLE = 4,
    AST_FUNCTION = 5,
    AST_ALIAS = 6,
    AST_EXPRESSION = 7,
    AST_CONDITIONAL = 8,

    // Types
    AST_IDENTIFIER = 9,
    AST_BINARY = 10,
    AST_STRING = 11,
    AST_LONG = 12,
    AST_DOUBLE = 13,
    AST_NULL = 14,

    // Operators
    AST_REFERENCE = 15,
    AST_OPERATOR = 16
};


/******************************************************************************
 *                             ScopeAST                                       *
 ******************************************************************************/
class ScopeAST {

    public:
        std::string name;                                                       // Useful for debugging.
        ASTType type = AST_SCOPE;                                               // Used to determine which << operator to use.
        int depth;                                                              // This is useful for tree building.
        bool is_static;                                                         // This is useful for semantic checking.
        std::shared_ptr<ScopeAST> parent_scope = nullptr;                       // It is necessary to know parent for semantic checking.
        std::vector<std::shared_ptr<ScopeAST>> members_by_order;                // Execution happens in order here.
        std::map<std::string, std::shared_ptr<ScopeAST>> members_by_name;       // Used for building the reference connections.
        
        ScopeAST(
            std::string name,
            int depth,
            bool is_static
        ) :
            name(name),
            depth(depth),
            is_static(is_static)
        {}

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

};
std::ostream& operator<<(std::ostream&, const ScopeAST&);


/******************************************************************************
 *                               RawAST                                       *
 ******************************************************************************/
class RawAST : public ScopeAST {

    public:
        std::string code;
        
        RawAST(
            std::string name,
            std::string code,
            int depth
        ) :
            ScopeAST(name, depth, false),
            code(code)
        {
            type = AST_RAW;
        }

};
std::ostream& operator<<(std::ostream&, const RawAST&);


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

};
std::ostream& operator<<(std::ostream&, const VariableAST&);


/******************************************************************************
 *                             FunctionAST                                    *
 ******************************************************************************/
class FunctionAST : public ScopeAST {

    public:
        bool takes_varargs;
        std::vector<std::string> parameters_by_order;
        std::map<std::string, std::shared_ptr<VariableAST>> parameters_by_name;

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

};
std::ostream& operator<<(std::ostream&, const FunctionAST&);


/******************************************************************************
 *                             ExpressionAST                                  *
 ******************************************************************************/
class ExpressionAST : public ScopeAST {

    public:
        std::shared_ptr<ExpressionAST> next = nullptr;

        ExpressionAST(
            int depth
        ) :
            ScopeAST("", depth, false)
        {
            type = AST_EXPRESSION;
        }

};
std::ostream& operator<<(std::ostream&, const ExpressionAST&);


/******************************************************************************
 *                                AliasAST                                    *
 ******************************************************************************/
class AliasAST : public ScopeAST {

    public:
        std::shared_ptr<ExpressionAST> expression = nullptr;

        AliasAST(
            std::string name,
            int depth
        ) :
            ScopeAST(name, depth, true)
        {
            type = AST_ALIAS;
        }

};
std::ostream& operator<<(std::ostream&, const AliasAST&);


/******************************************************************************
 *                            ConditionalAST                                  *
 ******************************************************************************/
class ConditionalAST : public AliasAST {

    public:
        std::string condition;
        std::shared_ptr<ScopeAST> when_false = nullptr;

        ConditionalAST(
            std::string condition,
            int depth
        ) :
            AliasAST("", depth),
            condition(condition)
        {
            type = AST_CONDITIONAL;
        }

};
std::ostream& operator<<(std::ostream&, const ConditionalAST&);


/******************************************************************************
 *                             IdentifierAST                                  *
 ******************************************************************************/
class IdentifierAST : public ExpressionAST {

    public:
        // Ignoring for the moment some targets will be calculated.
        std::shared_ptr<ScopeAST> target = nullptr;

        IdentifierAST(
            std::string name,
            int depth
        ) :
            ExpressionAST(depth)
        {
            type = AST_IDENTIFIER;
            this->name = name;
        }

};
std::ostream& operator<<(std::ostream&, const IdentifierAST&);


/******************************************************************************
 *                                StringAST                                   *
 ******************************************************************************/
class StringAST : public ExpressionAST {

    public:
        std::string value;
        
        StringAST(
            std::string value,
            int depth
        ) :
            ExpressionAST(depth),
            value(value)
        {
            type = AST_STRING;
        }

};
std::ostream& operator<<(std::ostream&, const StringAST&);


/******************************************************************************
 *                                BinaryAST                                   *
 ******************************************************************************/
class BinaryAST : public ExpressionAST {

    public:
        std::vector<char> value;
        
        BinaryAST(
            std::string value,
            int depth
        ) :
            ExpressionAST(depth)
        {
            type = AST_BINARY;
            // TODO: convert value to value!
        }

};
std::ostream& operator<<(std::ostream&, const BinaryAST&);


/******************************************************************************
 *                                 LongAST                                    *
 ******************************************************************************/
class LongAST : public ExpressionAST {

    public:
        long value;
        
        LongAST(
            long value,
            int depth
        ) :
            ExpressionAST(depth),
            value(value)
        {
            type = AST_LONG;
        }

};
std::ostream& operator<<(std::ostream&, const LongAST&);


/******************************************************************************
 *                                DoubleAST                                   *
 ******************************************************************************/
class DoubleAST : public ExpressionAST {

    public:
        double value;
        
        DoubleAST(
            double value,
            int depth
        ) :
            ExpressionAST(depth),
            value(value)
        {
            type = AST_DOUBLE;
        }

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

};
std::ostream& operator<<(std::ostream&, const NullAST&);


/******************************************************************************
 *                              OperatorAST                                   *
 ******************************************************************************/
class OperatorAST : public ExpressionAST {

    public:
        std::string op;
        bool is_self;

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

};
std::ostream& operator<<(std::ostream&, const OperatorAST&);


/******************************************************************************
 *                              ReferenceAST                                  *
 ******************************************************************************/
class ReferenceAST : public OperatorAST {

    public:
        // Ignoring for the moment some targets will be calculated.
        std::shared_ptr<ExpressionAST> expression = nullptr;

        ReferenceAST(
            bool is_self,
            int depth
        ) :
            OperatorAST("", is_self, depth)
        {
            type = AST_REFERENCE;
        }

};
std::ostream& operator<<(std::ostream&, const ReferenceAST&);

