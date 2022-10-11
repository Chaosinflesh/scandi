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



class AliasAST : public DeclaredCitizenAST {
    
    public:
        // This will need to be linked from members during semantic analysis.
        std::shared_ptr<DeclaredCitizenAST> target;

        AliasAST(
            std::string name,
            int depth,
            std::shared_ptr<CitizenAST> scope
        ) :
            // Static-ness will be determined during semantic analysis.
            DeclaredCitizenAST(name, TOK_IDENTIFIER_ALIAS, depth, false, scope) {
            printId = CITIZEN_ALIAS;
            target = std::shared_ptr<DeclaredCitizenAST>(nullptr);
        }
};
std::ostream& operator<<(std::ostream&, const AliasAST&);


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


class ExpressionAST : public CitizenAST {
    public:
        std::vector<CitizenAST> stack;
    
        ExpressionAST(
            int depth,
            std::shared_ptr<CitizenAST> scope
        ) :
            CitizenAST(depth, false, scope)
        {
            printId = CITIZEN_EXPRESSION;
        }
}
std::ostream& operator<<(std::ostream&, const ExpressionAST&);
