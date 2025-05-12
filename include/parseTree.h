#ifndef PARSETREE_H
#define PARSETREE_H

#include <vector>
#include <memory>

#include "token.h"

struct Node{
    NodeType type;
    NodeSubType subtype = NodeSubType::none;
    std::shared_ptr<Token> token = nullptr;
    std::vector<std::shared_ptr<Node>> children = {};
    Node(NodeType _type);
    Node(NodeType _type, std::shared_ptr<Token>);
    void print(int depth=0);
};

enum class ErrorType{
    MAX_DEPTH,
    INVALID_ARGUMENT,
    EXPECTED_STATEMENT,
    UNEXPECTED_EOF,
    SYNTAX_ERROR,
    NONE,
};

const std::string ErrorTypeStrings[] = {
    "MAX_DEPTH",
    "INVALID_ARGUMENT",
    "EXPECTED_STATEMENT",
    "UNEXPECTED_EOF",
    "SYNTAX_ERROR",
    "NONE",
};

struct StackTrace{
    std::shared_ptr<Node> node;
    bool success = false;
    ErrorType errType = ErrorType::NONE;
    std::string err_s = "";
    std::vector<std::shared_ptr<StackTrace>> children = {};
    StackTrace();
    StackTrace(std::shared_ptr<Node>);
    StackTrace(ErrorType, std::string);
};

struct parseTreeReturn{
    std::vector<StackTrace> traces;
    bool success = true;
};

parseTreeReturn createParseTree(std::vector<Token>&);

#endif