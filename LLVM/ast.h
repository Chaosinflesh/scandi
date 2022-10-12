// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0
#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include "lexer.h"


#define CITIZEN_UNIDENTIFIED   0
#define CITIZEN_DECLARED       1
#define CITIZEN_ALIAS          2
#define CITIZEN_FUNCTION       3
#define CITIZEN_EXPRESSION     4
#define CITIZEN_GOTO           5
#define CITIZEN_OPERATOR       6
#define CITIZEN_STRING         7
#define CITIZEN_INTEGER        8
#define CITIZEN_FLOAT          9
#define CITIZEN_COMPARATOR    10
#define CITIZEN_NULL          11


class CitizenAST {

    public:
        int printId = CITIZEN_UNIDENTIFIED;
        int depth;
        bool isStatic = false;
        std::shared_ptr<CitizenAST> scope;
        std::vector<std::shared_ptr<CitizenAST>> members;

        CitizenAST(
            int depth,
            bool isStatic,
            std::shared_ptr<CitizenAST> scope
        ) :
            depth(depth),
            isStatic(isStatic),
            scope(scope)
        {}

        virtual ~CitizenAST() {}

        friend std::ostream& operator<<(std::ostream&, const CitizenAST&);
};
std::ostream& operator<<(std::ostream&, const CitizenAST&);


class ExpressionAST : public CitizenAST {
    public:
        std::vector<std::shared_ptr<CitizenAST>> stack;
    
        ExpressionAST(
            int depth,
            std::shared_ptr<CitizenAST> scope
        ) :
            CitizenAST(depth, false, scope)
        {
            printId = CITIZEN_EXPRESSION;
        }
};
std::ostream& operator<<(std::ostream&, const ExpressionAST&);


class DeclaredCitizenAST : public CitizenAST {
    public:
        std::string name;
        TokenType type;

        DeclaredCitizenAST(
            std::string name,
            TokenType type,
            int depth,
            bool isStatic,
            std::shared_ptr<CitizenAST> scope
        ) : 
            CitizenAST(depth, isStatic, scope),
            name(name),
            type(type)
        {
            printId = CITIZEN_DECLARED;
        }
        
        friend std::ostream& operator<<(std::ostream&, const DeclaredCitizenAST&);
};
std::ostream& operator<<(std::ostream&, const DeclaredCitizenAST&);


class AliasAST : public ExpressionAST {
    
    public:
        std::string name;
        // This will need to be linked from members during semantic analysis.
        std::shared_ptr<CitizenAST> target;

        AliasAST(
            std::string name,
            int depth,
            std::shared_ptr<CitizenAST> scope
        ) :
            // Static-ness will be determined during semantic analysis.
            ExpressionAST(depth, scope),
            name(name)
        {
            printId = CITIZEN_ALIAS;
            target = std::shared_ptr<CitizenAST>(nullptr);
        }
};
std::ostream& operator<<(std::ostream&, const AliasAST&);


class OperatorAST : public CitizenAST {
        
    public:
        TokenType type;
        
        OperatorAST(
            int depth,
            TokenType type,
            std::shared_ptr<CitizenAST> scope
        ) :
            CitizenAST(depth, false, scope),
            type(type)
        {
            printId = CITIZEN_OPERATOR;
        }
};
std::ostream& operator<<(std::ostream&, const OperatorAST&);


class ComparatorAST : public CitizenAST {
        
    public:
        TokenType type;
        // These will be set during semantic analysis.
        std::shared_ptr<CitizenAST> ifTrue;
        std::shared_ptr<CitizenAST> ifFalse;
        
        ComparatorAST(
            int depth,
            TokenType type,
            std::shared_ptr<CitizenAST> scope
        ) :
            CitizenAST(depth, false, scope),
            type(type)
        {
            printId = CITIZEN_COMPARATOR;
            ifTrue = std::shared_ptr<CitizenAST>(nullptr);
            ifFalse = std::shared_ptr<CitizenAST>(nullptr);
        }
};
std::ostream& operator<<(std::ostream&, const ComparatorAST&);


class NullAST : public CitizenAST {
    
    public:
        NullAST(
            int depth,
            std::shared_ptr<CitizenAST> scope
        ) :
            CitizenAST(depth, false, scope)
        {
            printId = CITIZEN_NULL;
        }
};
std::ostream& operator<<(std::ostream&, const NullAST&);


class StringAST : public CitizenAST {
    
    public:
        std::string value;
        
        StringAST(
            std::string value,
            int depth,
            std::shared_ptr<CitizenAST> scope
        ) :
            CitizenAST(depth, false, scope),
            value(value)
        {
            printId = CITIZEN_STRING;
        }
};
std::ostream& operator<<(std::ostream&, const StringAST&);


class IntegerAST : public CitizenAST {
    
    public:
        long value;
        
        IntegerAST(
            long value,
            int depth,
            std::shared_ptr<CitizenAST> scope
        ) :
            CitizenAST(depth, false, scope),
            value(value)
        {
            printId = CITIZEN_INTEGER;
        }
};
std::ostream& operator<<(std::ostream&, const IntegerAST&);


class FloatAST : public CitizenAST {
    
    public:
        double value;
        
        FloatAST(
            double value,
            int depth,
            std::shared_ptr<CitizenAST> scope
        ) :
            CitizenAST(depth, false, scope),
            value(value)
        {
            printId = CITIZEN_FLOAT;
        }
};
std::ostream& operator<<(std::ostream&, const FloatAST&);


class FunctionAST : public DeclaredCitizenAST {
    
    public:
        bool takesVarargs = false;
        std::vector<std::shared_ptr<CitizenAST>> arguments;

        FunctionAST(
            std::string name,
            bool isStatic,
            bool takesVarargs,
            int depth,
            std::shared_ptr<CitizenAST> scope
        ) :
            DeclaredCitizenAST(
                name,
                TOK_DECLARATION_FUNCTION,
                depth,
                isStatic,
                scope
            ),
            takesVarargs(takesVarargs)
        {
            printId = CITIZEN_FUNCTION;
        }
};
std::ostream& operator<<(std::ostream&, const FunctionAST&);


class GotoAST : public AliasAST {
    public:
        GotoAST(
            std::string name,
            int depth,
            std::shared_ptr<CitizenAST> scope
        ) : 
            AliasAST(name, depth, scope)
        {
            printId = CITIZEN_GOTO;
        }
};
std::ostream& operator<<(std::ostream&, const GotoAST&);
