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
    auto n = next;
    while (n) {
        if (n->type == AST_VARIABLE && n->name == name) {
            return true;
        }
        n = n->next;
    }
    return false;
}


bool AST::can_see(const string name) {
    if (has_member(name)) {
        return true;
    } else if (parent) {
        return parent->can_see(name);
    }
    return false;
}


SHARED(AST) AST::get_member(const string name) {
    for (auto c: children) {
        if (c->name == name) {
            return c;
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
    return CHAR_STR(type) + std::to_string(properties) + "." + vis;
}


ostream& operator<<(ostream& os,  const SHARED(AST)& ast) {
    if (ast->type < 'a') {
        if (ast->depth >= 0) {
            os << endl << string(ast->depth, ' ') << ast->shorthand();
        }
    } else {
        os << " " << ast->shorthand();
    }
    // This is for references.
    if (ast->type == AST_OPERATOR && ast->alt) {
        os << "[( " << ast->alt << " )]";
    }
    if (ast->next) {
        os << ast->next;
    }
    if (!ast->children.empty()) {
        // TODO: Name the parts.
        for (auto c: ast->children) {
            os << c;
        }
    }
    if (ast->type == AST_CONDITIONAL && ast->alt) {
        os << ast->alt;
    }
    return os;
}


