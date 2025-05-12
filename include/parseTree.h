#ifndef PARSETREE_H
#define PARSETREE_H

#include <vector>

#include "token.h"

struct Node{
    NodeType type;
    NodeSubType subtype = NodeSubType::none;
    Token* token = nullptr;
    std::vector<Node*> children = {};
    Node(NodeType _type);
    Node(NodeType _type, Token &_token);
    ~Node();
    void print(int depth=0);
};

struct stackTrace{
    Node* node;
    std::string err = "";
    bool success = true;
    stackTrace* child = nullptr;
};

struct parseTreeReturn{
    std::vector<Node*> trees;
    bool success = true;
};

parseTreeReturn createParseTree(std::vector<Token>&);

#endif