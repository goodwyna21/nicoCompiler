#ifndef TOKENIZE_CPP
#define TOKENIZE_CPP

#include <vector>
#include <iostream>

#include "tokenize.h"

void cleanTokens(std::vector<Token> &tokens){
    int lineNo = 1;

    for(std::vector<Token>::iterator it = tokens.begin(); it != tokens.end(); ++it){
        if(it->type == TokenType::NEWLINE){
            lineNo++;
        }
        it->lineNumber = lineNo;
    }
    std::erase_if(tokens, [](Token &t){return t.type==TokenType::NEWLINE;});
}

tokenRet tokenize(const std::string& str){
    tokenRet ret; 
    std::string charBuffer = "";
    std::string binaryOperatorChars = "+-*/%=<>";
    std::string repeatedOperators = "+-<>";
    int lineNumber = 1;
    
    for(int i = 0; i < str.size(); i++){
        charBuffer = "";
        char c = str.at(i);

        //multiline comments
        if(c == '/' && i+2 < str.size() && str.at(i+1) == '/' && str.at(i+2) == '/'){
            int startLine = lineNumber;
            i+=2;
            for(; i+2 < str.size() && (str.at(i) != '/' || str.at(i+1) != '/' || str.at(i+2) != '/'); i++){
                if(str.at(i) == '\n'){ ret.tokens.push_back(Token(TokenType::NEWLINE)); lineNumber++; }
            }; //skip to end of multiline
            if(i == str.size() - 2){
                return tokenRet(ErrorType::SYNTAX_ERROR, "expected end of multiline comment", startLine);
            }
            i+=2;
            continue;
        }

        //single line comments
        if(c == '/' && i+1 < str.size() && str.at(i+1) == '/'){
            for(; i < str.size() && str.at(i) != '\n'; i++); //skip to end of line
            i--;
            continue;
        }

        if(c == ';'){ ret.tokens.push_back(Token(TokenType::SEMI)); continue; }
        if(c == '\n'){ ret.tokens.push_back(Token(TokenType::NEWLINE)); lineNumber++; continue; }
        if(c == ' ' || c == '\t'){ /*skip whitespace characters*/ continue; }
        //handle ()[]{}
        if(c == '('){ ret.tokens.push_back(Token(TokenType::OPEN_PARENTH)); continue; }
        if(c == ')'){ ret.tokens.push_back(Token(TokenType::CLOSE_PARENTH)); continue; }
        if(c == '['){ ret.tokens.push_back(Token(TokenType::OPEN_BRACKET)); continue; }
        if(c == ']'){ ret.tokens.push_back(Token(TokenType::CLOSE_BRACKET)); continue; }
        if(c == '{'){ ret.tokens.push_back(Token(TokenType::OPEN_CURLY)); continue; }
        if(c == '}'){ ret.tokens.push_back(Token(TokenType::CLOSE_CURLY)); continue; }
        //handle +-*/%=<>
        if(binaryOperatorChars.find(c) != std::string::npos){
            //handle += type shit
            if(i + 1 < str.size() && str.at(i+1) == '='){
                ret.tokens.push_back(Token(TokenType::BINARY_OPERATOR, charBuffer + c + "="));
                i++;
                continue; 
            }
            //handle ++, --, <<, >>
            if(i + 1 < str.size() && repeatedOperators.find(c) != std::string::npos && str.at(i+1) == c){ 
                ret.tokens.push_back(Token((c == '+' || c == '-') ? TokenType::UNARY_OPERATOR : TokenType::BINARY_OPERATOR, charBuffer + c + c));
                i++;
                continue;
            //must be +-*/%=<>
            }
            ret.tokens.push_back(Token(TokenType::BINARY_OPERATOR, std::string(1, c)));
            continue;
        }
        //handle !, !=, &, &&, |, ||
        if(c == '!' || c == '&' || c == '|'){
            //check for !=, &&, ||
            if(i + 1 < str.size()){
                if(c == '!'){
                    if(str.at(i+1) == '='){ 
                        ret.tokens.push_back(Token(TokenType::BINARY_OPERATOR, "!=")); 
                        i++; 
                        continue; 
                    }
                    ret.tokens.push_back(Token(TokenType::UNARY_OPERATOR, "!")); 
                    continue;
                }
                if(str.at(i+1) == c){ 
                    ret.tokens.push_back(Token(TokenType::BINARY_OPERATOR, c=='&' ? "&&" : "||")); 
                    i++; 
                    continue; 
                }
                ret.tokens.push_back(Token(TokenType::BINARY_OPERATOR, std::string(1, c))); 
                continue;
            }
            //must be !, &, |
            if(c == '!'){ 
                ret.tokens.push_back(Token(TokenType::UNARY_OPERATOR, "!")); 
                continue; 
            }
            //must be &, |
            ret.tokens.push_back(Token(TokenType::BINARY_OPERATOR, std::string(1, c))); 
            continue;
        }
        //start of identifier
        if(std::isalpha(c) || c == '_'){ 
            while(std::isalnum(str.at(i)) || str.at(i) == '_'){
                charBuffer += str.at(i);
                i++;
                if(i >= str.size()){
                    break;
                }
            }
            i--;
            
            //handle different identifiers
            if(charBuffer == "return"){
                ret.tokens.push_back(Token(TokenType::RESERVED));
                ret.tokens.back().s_value = "return";
                continue; 
            }
            //generic identifier (variable/function names etc.)
            ret.tokens.push_back(Token(TokenType::IDENTIFIER, charBuffer));
            continue;
        }
        //number literal
        if(std::isdigit(c) || c == '.'){ 
            while((str.at(i) >= 48 && str.at(i) <= 57) || str.at(i) == '.'){
                charBuffer += str.at(i);
                i++;
                if(i >= str.size()){
                    break;
                }
            }
            i--;
            
            if(charBuffer.find('.') == std::string::npos){ // integer
                ret.tokens.push_back(Token(TokenType::INT_LITERAL, std::stoi(charBuffer)));
                continue;
            }
            //floating point goes here
        }
        //string literal
        if(c == '"'){
            for(; str.at(i) != '"'; i++){
                if(i + 1 >= str.size()){

                    break;
                }
            }
        }
    }
    
    cleanTokens(ret.tokens);
    return ret;
}

void printTokens(std::vector<Token>& tokens){
    int curLine = 1;
    for(std::vector<Token>::iterator it = tokens.begin(); it != tokens.end(); ++it){
        if(it->lineNumber != curLine){
            std::cout << "\n";
            curLine = it->lineNumber;
        }
        std::cout << "[";
        std::cout << it->type;
        if(it->has_i_val){
            std::cout << " = " << it->i_value;
        }
        if(it->has_s_val){
            std::cout << " = " << it->s_value;
        }
        std::cout << "] ";
    }
}

#endif