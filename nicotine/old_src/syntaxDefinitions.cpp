#define DONTRUNTHIS
#ifndef DONTRUNTHIS

#ifndef SYNTAX_DEFINITIONS_CPP
#define SYNTAX_DEFINITIONS_CPP


#include <vector>
#include <map>
#include "token.h"

std::map<NodeType, std::vector<std::vector<NodeType>>> syntaxDefinition = {
    {NodeType::statement, {
        //function call syntax goes here, special case
        {NodeType::open_parenth, NodeType::statement, NodeType::close_parenth}, // (2+3)
        {NodeType::unary_operator, NodeType::operand}, //++x
        {NodeType::operand, NodeType::unary_operator}, //x++
        {NodeType::operand, NodeType::binary_operator, NodeType::operand} //x+2
    }},{NodeType::operand, {
        {NodeType::variable}, //x
        {NodeType::value}, //2
        {NodeType::statement}
    }},{NodeType::variable, {
        {NodeType::identifier}, //x
        {NodeType::variable, NodeType::open_bracket, NodeType::operand, NodeType::close_bracket} //x[2]
    }}
};

std::map<NodeType, bool> hasSimpleSyntaxDefinition = {
    {NodeType::value, true},
    {NodeType::open_parenth, true},
    {NodeType::close_parenth, true},
    {NodeType::open_bracket, true},
    {NodeType::close_bracket, true},
    {NodeType::unary_operator, true},
    {NodeType::binary_operator, true},
    {NodeType::identifier, true}
};

#endif

#endif