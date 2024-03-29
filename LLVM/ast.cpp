// Scandi: ast.cpp
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#include "ast.h"
#include "globals.h"


bool AST::has_member(const string name) {
    for (auto c: children) {
        if (c->name == name) {
            return true;
        }
    }

    // Account for named parameters also.
    if (type == AST_FUNCTION) {
        auto n = next;
        while (n) {
            if (n->type == AST_VARIABLE && n->name == name) {
                return true;
            }
            n = n->next;
        }
    }
    return false;
}


bool AST::can_see(const string name) {
    if (has_member(name)) {
        return true;
    } else if (parent) {
        return parent->can_see(name);
    }
    // Only really applies at the global scope.
    return (this->name == name);
}


SHARED(AST) AST::get_member(const string name) {
    for (auto c: children) {
        if (c->name == name) {
            return c;
        }
    }
    // Account for named parameters also.
    if (type == AST_FUNCTION) {
        auto n = next;
        while (n) {
            if (n->type == AST_VARIABLE && n->name == name) {
                return n;
            }
            n = n->next;
        }
    }
    if (parent) {
        return parent->get_member(name);
    }
    return nullptr;
}


SHARED(AST) AST::get_correct_parent(SHARED(AST) member, SHARED(AST) parent) {
    while (member->depth <= parent->depth) {
        parent = parent->parent;
    }
    return parent;
}


bool AST::get_property(const char prop) {
    return properties & prop;
}


void AST::set_property(const char prop) {
    properties |= prop;
}


const string AST::shorthand() const {
    string vis = name;
    if (type == AST_LONG) {
        vis = std::to_string(numeric_value.l);
    } else if (type == AST_DOUBLE) {
        vis = std::to_string(numeric_value.d);
    }
    if (type == AST_IDENTIFIER) {
        vis += "->";
        if (alt) {
            vis += alt->shorthand();
        }
    }
    return CHAR_STR(type) + std::to_string(properties) + "." + vis;
}


ostream& line_stream(ostream& os,  const SHARED(AST)& ast, bool new_line) {
    if (ast->type < 'a') {
        if (ast->depth >= 0 && new_line) {
            os << endl << string(ast->depth, ' ') << ast->shorthand();
        }
    } else {
        os << " " << ast->shorthand();
    }
    if (ast->type == AST_REFERENCE && ast->alt) {
        os << "[( ";
        line_stream(os, ast->alt, false);
        os << " )]";
    }
    if (ast->next) {
        os << ast->next;
    }
    if (!ast->children.empty()) {
        for (auto c: ast->children) {
            os << c;
        }
    }
    if (ast->type == AST_CONDITIONAL && ast->alt) {
        os << ast->alt;
    }
    return os;
}


ostream& operator<<(ostream& os,  const SHARED(AST)& ast) {
    return line_stream(os, ast, true);
}
