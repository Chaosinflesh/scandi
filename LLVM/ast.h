// Scandi: ast.h
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#pragma once
#include "globals.h"
#include "lexer.h"


enum ASTType {
    AST_SCOPE =         'S',
    AST_RAW =           'R',

    // Structural
    AST_LABEL =         'L',
    AST_VARIABLE =      'V',
    AST_FUNCTION =      'F',
    AST_ALIAS =         'A',
    AST_EXPRESSION =    'E',
    AST_CONDITIONAL =   'C',

    // Types
    AST_IDENTIFIER =    'i',
    AST_BINARY =        'b',
    AST_STRING =        's',
    AST_LONG =          'l',
    AST_DOUBLE =        'd',
    AST_NULL =          'n',

    // Operators
    AST_REFERENCE =     'r',
    AST_OPERATOR =      'o'
};


class AST {

    public:
        enum ASTOptions {
            OPT_STATIC =       1,
            OPT_TARGETS_SELF = 2,
            OPT_HAS_VARARGS =  4
        };
        
        ASTType type = AST_SCOPE;     // Used to determine which << operator to use.
        string name;                  // Used for mapping.
        int depth;                    // This is useful for tree building.
        char properties;              // This is useful for semantic checking.
        SHARED(AST) parent = nullptr; // It is necessary to know parent for semantic checking.
        vector<SHARED(AST)> children; // Execution happens in order here.
        SHARED(AST) next = nullptr;   // For expressions, functions, conditionals.
        SHARED(AST) alt = nullptr;    // Extra scope information - such as RAW code
                                      // or when a conditional is false, or a link to the
                                      // target of an identifier.
        union {
            long l;
            double d;
        } numeric_value;              // Obv holds these types of values.
        
        AST(
            ASTType type,
            string name,
            int depth,
            bool is_static
        ) :
            type(type),
            name(name),
            depth(depth)
        {
            properties = is_static ? OPT_STATIC : 0;
        }
        virtual ~AST() {}

        bool has_member(const string) ;
        bool can_see(const string) ;
        SHARED(AST) get_member(const string) ;

        // Returns the parent the provided AST should use (based on depth).
        static SHARED(AST) get_correct_parent(SHARED(AST), SHARED(AST));

        // Checks if the specified property is set.
        bool get_property(const char);
        void set_property(const char);

        // Get the shorthand version of this AST for convenient display.
        const string shorthand() const;

};
ostream& operator<<(ostream&, const SHARED(AST)&);

