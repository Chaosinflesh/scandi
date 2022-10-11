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


class CitizenAST {

    public:
        int printId = CITIZEN_UNIDENTIFIED;
        int depth;
        std::shared_ptr<CitizenAST> scope;
        std::vector<std::shared_ptr<CitizenAST>> members;

        CitizenAST(int depth, std::shared_ptr<CitizenAST> scope) : depth(depth), scope(scope) {}

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
            std::shared_ptr<CitizenAST> scope
        ) : 
            CitizenAST(depth, scope),
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
        // This will need to be linked during semantic checking.
        std::vector<Token> link;
        std::shared_ptr<DeclaredCitizenAST> target;

        AliasAST(
            std::string name,
            std::vector<Token> link,
            int depth,
            std::shared_ptr<CitizenAST> scope
        ) :
            DeclaredCitizenAST(name, TOK_IDENTIFIER_ALIAS, depth, scope),
            link(link)
        {
            printId = CITIZEN_ALIAS;
            target = std::shared_ptr<DeclaredCitizenAST>(nullptr);
        }
};
std::ostream& operator<<(std::ostream&, const AliasAST&);


/*
class ReferenceAST : public CitizenAST {
        std::unique_ptr<DeclaredCitizenAST> target;
    
    public:
        ReferenceAST(std::unique_ptr<DeclaredCitizenAST> target) : target(target) {}
}
std::ostream& operator<<(std::ostream&, const ReferenceAST&);
*/
