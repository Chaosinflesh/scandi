// Scandi: scandi.cpp
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "semantics.h"


#define SCANDI_VERSION 0.1


void get_version() {
    std::cout << "scandi " << SCANDI_VERSION << std::endl;
}


void get_help() {
    std::cout << "scandi [options] <file1> [<file2> ...] [-o outfile]" << std::endl;
    std::cout << "options:" << std::endl;
    std::cout << "    --version             Get the current version" << std::endl;
    std::cout << "    --help                Display this help" << std::endl;
    std::cout << "    --libdir <libdir>     Get the current version" << std::endl;
    std::cout << "    -o <outfile>          Specify the executable name" << std::endl;
}


std::string LIB_DIR = "stdlib";
std::string OUTPUT = "a.out";
std::vector<std::string> FILENAMES;
std::vector<std::string> LIBFILES;


void get_library_files() {
}





int run() {
    auto global = std::make_shared<ScopeAST>("global", -1, true);

    for (auto f: FILENAMES) {
        std::vector<Token> tokens;

        // 1. Tokenize
        std::ifstream file(f);
        auto name = f.substr(f.find_last_of("/") + 1, f.length() - f.find_last_of(".scandi"));
        if (!tokenize_stream(tokens, file, name)) {
            std::cerr << "LEXING FAILED" << std::endl;
            return 1;
        }
        file.close();

        // 2. Parse
        if (!parse_to_ast(tokens, global)) {
            std::cerr << "PARSING FAILED" << std::endl;
            return 1;
        }
    }

    // 3. Semantic analysis
    //    TODO: Add in stdlib here
    global = analyse_semantics(global);

#ifdef DEBUG_AST
    std::cerr << *global << std::endl;
#endif

    return 0;
}


int main(const int argc, const char* argv[]) {

    // Arguments accepted:
    // --version
    // --help
    // --libdir <libdir>
    // -o output
    // all other arguments presumed imput files
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--version") == 0) {
            get_version();
        } else if (std::strcmp(argv[i], "--help") == 0) {
            get_help();
        } else if (std::strcmp(argv[i], "--libdir") == 0) {
            if (i + 1 < argc) {
                LIB_DIR = argv[i + 1];
            } else {
                std::cerr << "Invalid argument, libs directory expected" << std::endl;
            }
            i++;
        } else if (std::strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                OUTPUT = argv[i + 1];
            } else {
                std::cerr << "Invalid argument, outfile expected" << std::endl;
            }
            i++;
        } else {
            FILENAMES.push_back(argv[i]);
        }
    }

    if (FILENAMES.empty()) {
        std::cerr << "No input files" << std::endl;
        return 1;
    }

    get_library_files();

#ifdef DEBUG
    std::cout << std::endl << "Processing:";
    for (auto f : FILENAMES) {
        std::cout << std::endl << "    " << f;
    }
    std::cout << std::endl << "Libraries:";
    for (auto l : LIBFILES) {
        std::cout << std::endl << "    " << l;
    }
    std::cout << std::endl;
#endif

    return run();
}
