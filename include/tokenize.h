#ifndef TOKENIZE_H
#define TOKENIZE_H

#include <vector>
#include "token.h"

struct tokenRet{
    std::vector<Token> tokens;
    bool success = true;
    ErrorType et = ErrorType::NONE;
    std::string err_s = "";
    int errLine = -1;
    tokenRet(){}
    tokenRet(ErrorType _et, std::string _err_s, int _errLine){
        success = false;
        et = _et;
        err_s = _err_s;
        errLine = _errLine;
    }
};

tokenRet tokenize(const std::string&);
void printTokens(std::vector<Token>&);

#endif