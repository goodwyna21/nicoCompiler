#ifndef SYNTAX_DEFINITIONS_H
#define SYNTAX_DEFINITIONS_H

#include <vector>
#include <map>
#include "token.h"

std::map<NodeType, std::vector<std::vector<NodeType>>> syntaxDefinition;
std::map<NodeType, bool> hasSimpleSyntaxDefinition;

#endif