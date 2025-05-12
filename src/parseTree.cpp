#ifndef PARSETREE_CPP
#define PARSETREE_CPP

#include <vector>
#include <iostream>
#include <memory>

#include "token.h"
//#include "syntaxDefinitions.h"
#include "parseTree.h"

#define MAX_PARSE_DEPTH 10
#define PARSE_TREE_DEBUG_PRINT


Node::Node(NodeType _type) : type(_type) {}
Node::Node(NodeType _type, Token &_token) : type(_type), token(&_token) {}
Node::~Node(){
    for(int i = 0; i < children.size(); i++){
        delete children.at(i);
    }
}
void Node::print(int depth){
    if(depth > 0) std::cout << "└";
    for(int i = 0; i < depth; i++){
        std::cout << " -";
    }
    std::cout << "(" << type << ")";
    if(subtype != NodeSubType::none){
        std::cout << " - " << subtype;
    }
    if(token != nullptr){
        std::cout << " : " << token->type;
        if(token->has_i_val){
            std::cout << "(" << token->i_value << ")";
        }
        if(token->has_s_val){
            std::cout << "(\"" << token->s_value << "\")";
        }
    }
    std::cout << "\n";
    for(int i = 0; i < children.size(); i++){
        children.at(i)->print(depth+1);
    }
}

Node* parseParentheses(std::vector<Token>::iterator*, std::vector<Token>::iterator, int);
Node* parseOperand(std::vector<Token>::iterator*, std::vector<Token>::iterator, int);
Node* parseValue(std::vector<Token>::iterator*, std::vector<Token>::iterator, int);
Node* parseVariable(std::vector<Token>::iterator*, std::vector<Token>::iterator, int);
Node* parseArgument(std::vector<Token>::iterator*, std::vector<Token>::iterator, int);


Node* parseStatement(std::vector<Token>::iterator* it, std::vector<Token>::iterator end, int depth){
#ifdef PARSE_TREE_DEBUG_PRINT
    for(int i = 0; i < depth; i++){ std::cout << ".  "; }
    std::cout << "statement | [" << (*it)->type << "] - [" << end->type << "] |\n";
#endif
    if(depth > MAX_PARSE_DEPTH || (*it) == end){ return nullptr; }
    std::vector<Token>::iterator prevPosition = (*it);

    if((*it)->type == TokenType::RESERVED && (*it)->s_value == "return"){
        Node* returnStatement = new Node(NodeType::statement);
        returnStatement->subtype = NodeSubType::_return;

        ++(*it);
        Node* retOpSearch = parseStatement(it, end, depth+1);
        if(retOpSearch != nullptr){
            returnStatement->children.push_back(retOpSearch);
        } else {
            (*it) = prevPosition + 1;
            retOpSearch = parseOperand(it, end, depth+1);
            if(retOpSearch != nullptr){
                returnStatement->children.push_back(retOpSearch);
            }
        }
#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found return\n";
#endif
        return returnStatement;
    }

    //parentheses
    if((*it)->type == TokenType::OPEN_PARENTH){
        Node* parenthSearch = parseParentheses(it, end, depth+1);
        if(parenthSearch != nullptr){

#ifdef PARSE_TREE_DEBUG_PRINT
            for(int i = 0; i < depth; i++){ std::cout << ".  "; }
            std::cout << "found parentheses\n";
#endif
            return parenthSearch;
        }
        return nullptr;
    }
    (*it) = prevPosition; 

    //implement function call

    //prefix unary
    if((*it)->type == TokenType::UNARY_OPERATOR){
        ++(*it);
        Node* operandSearch = parseOperand(it, end, depth+1);
        if(operandSearch != nullptr){
            Node* ret = new Node(NodeType::statement);
            ret->subtype = NodeSubType::prefix_unary;
            ret->children.push_back(new Node(NodeType::_operator, *prevPosition));
            ret->children.push_back(operandSearch);

#ifdef PARSE_TREE_DEBUG_PRINT
            for(int i = 0; i < depth; i++){ std::cout << ".  "; }
            std::cout << "found prefix unary\n";
#endif

            return ret;
        }
        (*it) = prevPosition; 
    }

    //binary or postfix operator
    Node* operandSearch = parseOperand(it, end, depth+1);
    if(operandSearch != nullptr){
        std::vector<Token>::iterator op1endPosition = (*it);

        //binary op
        if((*it)->type == TokenType::BINARY_OPERATOR){
            ++(*it);
            Node* op2Search = parseOperand(it, end, depth+1);
            if(op2Search != nullptr){
                Node* ret = new Node(NodeType::statement);
                ret->subtype = NodeSubType::binary_op;
                ret->children.push_back(operandSearch);
                ret->children.push_back(new Node(NodeType::_operator, *op1endPosition));
                ret->children.push_back(op2Search);

#ifdef PARSE_TREE_DEBUG_PRINT
                for(int i = 0; i < depth; i++){ std::cout << ".  "; }
                std::cout << "found binary op\n";
#endif

                return ret;
            }

            return nullptr;
        }
        (*it) = op1endPosition;
        //postfix unary
        if((*it)->type == TokenType::UNARY_OPERATOR){
            Node* ret = new Node(NodeType::statement);
            ret->subtype = NodeSubType::postfix_unary;
            ret->children.push_back(operandSearch);
            ret->children.push_back(new Node(NodeType::_operator, **it));
            ++(*it);

#ifdef PARSE_TREE_DEBUG_PRINT
            for(int i = 0; i < depth; i++){ std::cout << ".  "; }
            std::cout << "found postfix unary\n";
#endif

            return ret;
        }
    }

    return nullptr;
}

Node* parseParentheses(std::vector<Token>::iterator* it, std::vector<Token>::iterator end, int depth){
#ifdef PARSE_TREE_DEBUG_PRINT
    for(int i = 0; i < depth; i++){ std::cout << ".  "; }
    std::cout << "parentheses | [" << (*it)->type << "] - [" << end->type << "] |\n";
#endif
    if(depth > MAX_PARSE_DEPTH || (*it) == end){ return nullptr; }
    std::vector<Token>::iterator startPosition = (*it);

    if((*it)->type != TokenType::OPEN_PARENTH){
        return nullptr;
    }
    ++(*it);
    int parenthCount = 1;

    while((*it) != end){
        if((*it)->type == TokenType::OPEN_PARENTH){
            parenthCount++;
        }
        if((*it)->type == TokenType::CLOSE_PARENTH){
            parenthCount--;
            if(parenthCount == 0){
                break;
            }
        }
        ++(*it);
    }
    if(*it == end){
        return nullptr;
    }
    ++(*it);

    std::vector<Token>::iterator subIt = startPosition + 1;
    std::vector<Token>::iterator subEnd = (*it)-1;
    Node* argSearch = parseArgument(&subIt, subEnd, depth+1);
    if(argSearch != nullptr){
        return argSearch;
    }

    (*it) = startPosition;
    return nullptr;
}

Node* parseOperand(std::vector<Token>::iterator* it, std::vector<Token>::iterator end, int depth){
#ifdef PARSE_TREE_DEBUG_PRINT
    for(int i = 0; i < depth; i++){ std::cout << ".  "; }
    std::cout << "operand | [" << (*it)->type << "] - [" << end->type << "] |\n";
#endif
    if(depth > MAX_PARSE_DEPTH || (*it) == end){ return nullptr; }
    std::vector<Token>::iterator prevPosition = (*it);
    Node* valSearch = parseValue(it, end, depth+1);
    if(valSearch != nullptr){ //value
        Node* ret = new Node(NodeType::operand);
        ret->children.push_back(valSearch);

#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found value\n";
#endif

        return ret;
    }
    (*it) = prevPosition;

    Node* varSearch = parseVariable(it, end, depth+1);
    if(varSearch != nullptr){ //variable
        Node* ret = new Node(NodeType::operand);
        ret->children.push_back(varSearch);

#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found variable\n";
#endif

        return ret;
    }
    (*it) = prevPosition;

    Node* stateSearch = parseStatement(it, end, depth+1);
    if(stateSearch != nullptr){ //statement
        Node* ret = new Node(NodeType::statement);
        ret->children.push_back(stateSearch);

#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found statement\n";
#endif

        return ret;
    }
    (*it) = prevPosition;

    return nullptr;
}

Node* parseArgument(std::vector<Token>::iterator* it, std::vector<Token>::iterator end, int depth){
#ifdef PARSE_TREE_DEBUG_PRINT
    for(int i = 0; i < depth; i++){ std::cout << ".  "; }
    std::cout << "argument | [" << (*it)->type << "] - [" << end->type << "] |\n";
#endif
    if(depth > MAX_PARSE_DEPTH || (*it) == end){ return nullptr; }

    if(end - (*it) < 1){
        return nullptr;
    }

    std::vector<Token>::iterator startPosition = (*it);

    //this has additional check to make sure that all tokens in parentheses are used
    Node* search = parseValue(it, end, depth+1);
    if(search != nullptr && (*it) == end){

#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found value\n";
#endif

        return search;
    }
    delete search;
    (*it) = startPosition;

    search = parseVariable(it, end, depth+1);
    if(search != nullptr && (*it) == end){

#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found variable\n";
#endif

        return search;
    }
    delete search;
    (*it) = startPosition;
    
    search = parseOperand(it, end, depth+1);
    if(search != nullptr && (*it) == end){

#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found operand\n";
#endif

        return search;
    }
    delete search;
    (*it) = startPosition;

    search = parseStatement(it, end, depth+1);
    if(search != nullptr && *it == end){

#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found statement\n";
#endif

        return search;
    }
    delete search;
    (*it) = startPosition;
    return nullptr;
}

Node* parseVariable(std::vector<Token>::iterator* it, std::vector<Token>::iterator end, int depth){
#ifdef PARSE_TREE_DEBUG_PRINT
    for(int i = 0; i < depth; i++){ std::cout << ".  "; }
    std::cout << "variable | [" << (*it)->type << "] - [" << end->type << "] |\n";
#endif
    if(depth > MAX_PARSE_DEPTH || (*it) == end){ return nullptr; }
    
    std::vector<Token>::iterator prevPosition = (*it);
    if((*it)->type == TokenType::IDENTIFIER){ //x
        Node* ret = new Node(NodeType::variable, (**it));
        ++(*it);

        std::vector<Token>::iterator identifierEndPosition = (*it);

        //check for a[b] notation
        while((*it) != end && (*it)->type == TokenType::OPEN_BRACKET){
            bool bracketFound = false;
            ++(*it);
            int bracketCount = 1;
            for(; (*it) != end; ++(*it)){
                if((*it)->type == TokenType::OPEN_BRACKET){
                    bracketCount++;
                }
                if((*it)->type == TokenType::CLOSE_BRACKET){
                    bracketCount--;
                    if(bracketCount == 0){
                        bracketFound = true;
                        break;
                    }
                }
            }
            if(!bracketFound){ //malformed brackets
                delete ret;
                return nullptr;
            }

            std::vector<Token>::iterator subIt = identifierEndPosition + 1;
            std::vector<Token>::iterator subEnd = *it;
            Node* argSearch = parseArgument(&subIt, subEnd, depth+1);
            if(argSearch != nullptr){
#ifdef PARSE_TREE_DEBUG_PRINT
                for(int i = 0; i < depth; i++){ std::cout << ".  "; }
                std::cout << "found index\n";
#endif  
                ret->children.push_back(argSearch);
                ++(*it);
                identifierEndPosition = (*it);
            } else {
                delete ret;
                return nullptr; //malformed argument
            }
        }

#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found " << ((ret->children.size() > 1) ? "a[b]" : "x") << "\n";
#endif
        if(ret->children.size() > 1){
            ret->subtype = NodeSubType::array_access;
        }
        return ret;
    }
    (*it) = prevPosition;
    return nullptr;
}

Node* parseValue(std::vector<Token>::iterator* it, std::vector<Token>::iterator end, int depth){
#ifdef PARSE_TREE_DEBUG_PRINT
    for(int i = 0; i < depth; i++){ std::cout << ".  "; }
    std::cout << "value | [" << (*it)->type << "] - [" << end->type << "] |\n";
#endif
    if(depth > MAX_PARSE_DEPTH || (*it) == end){ return nullptr; }
    if((*it)->type == TokenType::INT_LITERAL){
        Node* ret = new Node(NodeType::value, (**it));
        ++(*it);

#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found int literal\n";
#endif

        return ret;
    }
    return nullptr;
}

parseTreeReturn createParseTree(std::vector<Token>& tokens){
    std::vector<Node*> trees;
    std::vector<Token>::iterator lineStart = tokens.begin();
    std::vector<Token>::iterator lineEnd;
    for(std::vector<Token>::iterator it = tokens.begin(); it != tokens.end(); ++it){
        if(it->type == TokenType::SEMI){
            lineEnd = it;

            trees.push_back(parseStatement(&lineStart, lineEnd, 0));
            if(trees.back() == nullptr){
                return {{}, false};
            }

            lineStart = it+1;
        }
    }
    return {trees};
}

#endif