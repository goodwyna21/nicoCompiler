#ifndef TOKENIZE_CPP
#define TOKENIZE_CPP

#include <vector>
#include <iostream>
#include "token.h"

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

std::vector<Token> tokenize(const std::string& str){
    std::vector<Token> tokens;
    std::string charBuffer = "";
    std::string binaryOperatorChars = "+-*/%=<>";
    std::string repeatedOperators = "+-<>";
    
    for(int i = 0; i < str.size(); i++){
        charBuffer = "";
        char c = str.at(i);
        if(c == ';'){ tokens.push_back(Token(TokenType::SEMI)); }
        else if(c == '\n'){ tokens.push_back(Token(TokenType::NEWLINE)); }
        else if(c == ' ' || c == '\t'){ /*skip whitespace characters*/ }
        //handle ()[]{}
        else if(c == '('){ tokens.push_back(Token(TokenType::OPEN_PARENTH)); }
        else if(c == ')'){ tokens.push_back(Token(TokenType::CLOSE_PARENTH)); }
        else if(c == '['){ tokens.push_back(Token(TokenType::OPEN_BRACKET)); }
        else if(c == ']'){ tokens.push_back(Token(TokenType::CLOSE_BRACKET)); }
        else if(c == '{'){ tokens.push_back(Token(TokenType::OPEN_CURLY)); }
        else if(c == '}'){ tokens.push_back(Token(TokenType::CLOSE_CURLY)); }
        //handle +-*/%=<>
        else if(binaryOperatorChars.find(c) != std::string::npos){
            //handle += type shit
            if(i + 1 < str.size() && str.at(i+1) == '='){
                tokens.push_back(Token(TokenType::BINARY_OPERATOR, charBuffer + c + "="));
                i++;
            } else {
                //handle ++, --, <<, >>
                if(i + 1 < str.size() && repeatedOperators.find(c) != std::string::npos && str.at(i+1) == c){ 
                    tokens.push_back(Token((c == '+' || c == '-') ? TokenType::UNARY_OPERATOR : TokenType::BINARY_OPERATOR, charBuffer + c + c));
                    i++;
                //must be +-*/%=<>
                } else {
                    tokens.push_back(Token(TokenType::BINARY_OPERATOR, std::string(1, c)));
                }
            }
        }
        //handle !, !=, &, &&, |, ||
        else if(c == '!' || c == '&' || c == '|'){
            //check for !=, &&, ||
            if(i + 1 < str.size()){
                if(c == '!'){
                    if(str.at(i+1) == '='){ tokens.push_back(Token(TokenType::BINARY_OPERATOR, "!=")); i++;}
                    else { tokens.push_back(Token(TokenType::UNARY_OPERATOR, "!")); } 
                } else {
                    if(str.at(i+1) == c){ tokens.push_back(Token(TokenType::BINARY_OPERATOR, c=='&' ? "&&" : "||")); i++;}
                    else { tokens.push_back(Token(TokenType::BINARY_OPERATOR, std::string(1, c))); }
                }
            //must be !, &, |
            } else {
                if(c == '!'){ tokens.push_back(Token(TokenType::UNARY_OPERATOR, "!")); }
                else { tokens.push_back(Token(TokenType::BINARY_OPERATOR, std::string(1, c))); }
            }
        }
        //start of identifier
        else if(std::isalpha(c) || c == '_'){ 
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
                tokens.push_back(Token(TokenType::RESERVED));
                tokens.back().s_value = "return";
            } else { //generic identifier (variable/function names etc.)
                tokens.push_back(Token(TokenType::IDENTIFIER, charBuffer));
            }
        }
        //number literal
        else if(std::isdigit(c) || c == '.'){ 
            while((str.at(i) >= 48 && str.at(i) <= 57) || str.at(i) == '.'){
                charBuffer += str.at(i);
                i++;
                if(i >= str.size()){
                    break;
                }
            }
            i--;
            
            if(charBuffer.find('.') == std::string::npos){ // integer
                tokens.push_back(Token(TokenType::INT_LITERAL, std::stoi(charBuffer)));
            } else {
                //floating point goes here
            }
        }
    }
    
    cleanTokens(tokens);
    return tokens;
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