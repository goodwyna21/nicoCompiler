#ifndef TOKENIZE_H
#define TOKENIZE_H

#include <vector>
#include "token.h"

std::vector<Token> tokenize(const std::string&);
void printTokens(std::vector<Token>&);

#endif