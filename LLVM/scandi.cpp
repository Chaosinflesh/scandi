// Scandi: scandi.cpp
//
// Author: Neil Bradley
// Copyright: Neil Bradley
// License: GPL 3.0

#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <string>
#include "ast.h"
#include "codegen.h"
#include "globals.h"
#include "lexer.h"
#include "parser.h"
#include "semantics.h"


#define SCANDI_VERSION 0.1


bool debug_set = false;


void get_version() {
    std::cout << "scandi " << SCANDI_VERSION << std::endl;
}


void get_help() {
    std::cout << "scandi [options] <file1> [<file2> ...] [-o outfile]" << std::endl;
    std::cout << "options:" << std::endl;
    std::cout << "    --version             Get the current version" << std::endl;
    std::cout << "    --help                Display this help" << std::endl;
    std::cout << "    --debug               Set debug flag for verbose output" << std::endl;
    std::cout << "    --libdir <libdir>     Get the current version" << std::endl;
    std::cout << "    -o <outfile>          Specify the executable name" << std::endl;
}


std::string lib_dir = "./stdlib/";
std::string output = "a.out";
std::vector<std::string> in_files;
std::vector<std::string> lib_files;


void get_library_files() {
    DIR* dir;
    if ((dir = opendir(lib_dir.c_str())) != NULL) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG && std::string(entry->d_name).rfind(".scandi") != std::string::npos) {
                lib_files.push_back(lib_dir + entry->d_name);
            }
        }
        closedir (dir);
    } else {
        std::cerr << std::endl << "Unable to open lib_dir, program may not compile." << std::endl;
    }
}


int run() {
    auto global = SHARE(AST, AST_SCOPE, "global", -1, true);

    in_files.insert(in_files.begin(), lib_files.begin(), lib_files.end());

    for (auto f: in_files) {
        std::vector<Token> tokens;

        // 1. Tokenize
        std::ifstream file(f);
        auto name_start = f.find_last_of("/") + 1;
        auto name_end = f.find(".scandi", name_start);
        auto name = f.substr(name_start, name_end - name_start);
        if (!tokenize_stream(tokens, file, name)) {
            std::cerr << "LEXING FAILED" << std::endl;
            return 1;
        }
        file.close();

        // 2. Parse
        parse_to_ast(tokens, global);
    }
    DEBUG( "After parsing:" << std::endl << global << std::endl; )

    // 3. Semantic analysis
    analyse_semantics(global);
    DEBUG( "After semantics:" << std::endl << global << std::endl; )

    // 4. Generate LLVM IR
    generate_code(global);
    DEBUG( ""; )

    return 0;
}


int main( int argc,  char* argv[]) {

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
            
        } else if (std::strcmp(argv[i], "--debug") == 0) {
            debug_set = true;
            
        } else if (std::strcmp(argv[i], "--libdir") == 0) {
            if (i + 1 < argc) {
                lib_dir = argv[i + 1];
            } else {
                std::cerr << "Invalid argument, libs directory expected" << std::endl;
            }
            i++;
            
        } else if (std::strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output = argv[i + 1];
            } else {
                std::cerr << "Invalid argument, outfile expected" << std::endl;
            }
            i++;
            
        } else {
            in_files.push_back(argv[i]);
            
        }
    }

    if (in_files.empty()) {
        std::cerr << "No input files" << std::endl;
        return 1;
    }

    get_library_files();

    if (debug_set) {
        std::cout << std::endl << "Processing:";
        for (auto f : in_files) {
            std::cout << std::endl << "    " << f;
        }
        std::cout << std::endl << "Libraries: " << lib_dir;
        for (auto l : lib_files) {
            std::cout << std::endl << "    " << l;
        }
        std::cout << std::endl;
    }

    return run();
}
