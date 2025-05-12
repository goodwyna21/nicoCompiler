//  

/*
usage: nico [source].v [target].S 
assembler:  as -o [target].o [target].S
linker:     ld -macos_version_min 15.0.0 -o [target] [target].o -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` -e _start -arch arm64
running:    ./[target]


build with cmake --build build/
when updating cmakelists use cmake -S . -B build/


main.cpp
  - tokenize.h
  - parseTree.h
*/


/*
Working on:
  * changing parse functions to return a stack trace object, 
    that can track errors through calls.

    right now if you call "return ();" it simply returns (statement) - return,
    when it should be an error. this is because the call to find return's
    argument fails, but not because it doesnt exist but because its not valid

    adding the stack trace objects will allow this to be checked

  * operator chaining doesnt work, not even (1+2)+3
    this is because there is not check that all tokens in a line
    are used, and a similar schema to parseArgument should be used
    in statement
*/

#include <iostream>
#include <vector>

//used for reading source file
#include <fstream> 
#include <sstream>

#include "token.h"
#include "tokenize.h"
#include "parseTree.h"

int main(int argc, const char * argv[]) {
    if(argc < 3){
        std::cerr << "Incorrect usage. Correct usage is...\n";
        std::cerr << "nicotine [source].v [target].S\n";
        return EXIT_FAILURE;
    }

    std::string source_str;
    const char* fname = argv[1];
    
    std::ifstream file;
    file.open(fname);
    if(!file.is_open()){
        std::cerr << "Failed to open file \"" << fname << "\"\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    source_str = buffer.str();
    file.close();
    std::cout << "input source (" << fname << "):\n-----------------------------\n";
    std::cout << source_str;
    std::cout << "\n-----------------------------\n";
    
    std::vector<Token> tokens;
    {
        tokenRet tokenizeResult = tokenize(source_str);
        if(!tokenizeResult.success){
            std::cout << "Error: " << tokenizeResult.et << ", " << tokenizeResult.err_s << " on line " << tokenizeResult.errLine << "\n";
            return EXIT_FAILURE;
        }
        tokens = std::move(tokenizeResult.tokens);
    }

    std::cout << "tokens:\n-----------------------------\n";
    printTokens(tokens);
    std::cout << "\n-----------------------------\n\n";


    std::vector<std::shared_ptr<Node>> trees;
    {
        parseTreeReturn parseTree = createParseTree(tokens); 
        if(parseTree.success == false){
            std::cout << "Errors in creating parse tree:\n-----------------------------\n";
            for(auto it = parseTree.traces.begin(); it != parseTree.traces.end(); ++it){
                if(!it->success){
                    it->printTrace();
                }
            }
            std::cout << "-----------------------------\n";
        } 
        for(int i = 0; i < parseTree.traces.size(); i++){
            trees.push_back(std::move(parseTree.traces.at(i).node));
        }
    }

    std::cout << "\nparse tree:\n-----------------------------\n\n";
    for(int i = 0; i < trees.size(); i++){
        if(trees.at(i) == nullptr){ std::cout << "Error\n\n"; continue; }
        trees.at(i)->print(0);
        std::cout << "\n";
    }
    std::cout << "-----------------------------\n";

    /*
    std::cout << "assembly:\n-----------------------------\n";
    std::cout << compile(tokens);
    std::cout << "\n-----------------------------\n";
    */
    
    return EXIT_SUCCESS;
}
