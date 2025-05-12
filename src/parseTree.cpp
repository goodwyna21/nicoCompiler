#ifndef PARSETREE_CPP
#define PARSETREE_CPP

#include <vector>
#include <iostream>
#include <memory>

#include "parseTree.h"

#define MAX_PARSE_DEPTH 15
#define PARSE_TREE_DEBUG_PRINT


Node::Node(NodeType _type) : type(_type) {}
Node::Node(NodeType _type, std::shared_ptr<Token> _token) : type(_type), token(_token) {}
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

StackTrace::StackTrace() {
    node = nullptr;
    success = false;
}

StackTrace::StackTrace(std::shared_ptr<Node> _node){
    node = _node;
    success = true;
}
StackTrace::StackTrace(ErrorType et, std::string em, const std::vector<std::reference_wrapper<StackTrace>>& st_vec, bool _strongError){
    node = nullptr;
    success = false;
    errType = et;
    err_s = em;

    for(int i = 0; i < st_vec.size(); i++){
        children.push_back(std::make_shared<StackTrace>(st_vec.at(i)));
    }
    strongError = _strongError;
}

void StackTrace::printTrace(int depth){
    for(int i = 0; i < depth; i++){
        std::cout << " -";
    }
    std::cout << errType << ": " << err_s << ", " << children.size() << " children\n";
    for(auto it = children.begin(); it != children.end(); ++it){
        (*it)->printTrace(depth+1);
    }
}

StackTrace parseParentheses(std::vector<Token>::iterator*, std::vector<Token>::iterator, int);
StackTrace parseOperand(std::vector<Token>::iterator*, std::vector<Token>::iterator, int);
StackTrace parseValue(std::vector<Token>::iterator*, std::vector<Token>::iterator, int);
StackTrace parseVariable(std::vector<Token>::iterator*, std::vector<Token>::iterator, int);
StackTrace parseArgument(std::vector<Token>::iterator*, std::vector<Token>::iterator, int);

StackTrace parseStatement(std::vector<Token>::iterator* it, std::vector<Token>::iterator end, int depth){
#ifdef PARSE_TREE_DEBUG_PRINT
    for(int i = 0; i < depth; i++){ std::cout << ".  "; }
    std::cout << "statement | [" << (*it)->type << "] - [" << end->type << "] |\n";
#endif
    if(depth > MAX_PARSE_DEPTH || (*it) == end){ return StackTrace(ErrorType::MAX_DEPTH, "", {}); }
    std::vector<Token>::iterator prevPosition = (*it);

    if((*it)->type == TokenType::RESERVED && (*it)->s_value == "return"){
        StackTrace returnStatement = StackTrace(std::make_shared<Node>(NodeType::statement));
        returnStatement.node->subtype = NodeSubType::_return;

        ++(*it);
        StackTrace retStateSearch = parseStatement(it, end, depth+1);
        if(retStateSearch.success){
            returnStatement.node->children.push_back(std::move(retStateSearch.node));
        } else { 
            if(retStateSearch.strongError){
                (*it) = prevPosition;
                return retStateSearch;
            }
            (*it) = prevPosition + 1;
            StackTrace retOpSearch = parseOperand(it, end, depth+1);
            if(retOpSearch.success){
                returnStatement.node->children.push_back(std::move(retOpSearch.node));
            }
            if(retOpSearch.strongError){
                (*it) = prevPosition;
                return retOpSearch;
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
        StackTrace parenthSearch = parseParentheses(it, end, depth+1);
        if(parenthSearch.success){

#ifdef PARSE_TREE_DEBUG_PRINT
            for(int i = 0; i < depth; i++){ std::cout << ".  "; }
            std::cout << "found parentheses\n";
#endif
            return parenthSearch;
        }
        StackTrace ret(ErrorType::INVALID_ARGUMENT, "bad inside of parentheses", {parenthSearch}, true);
        (*it) = prevPosition;
        return ret;
    }
    (*it) = prevPosition; 

    //implement function call

    //prefix unary
    if((*it)->type == TokenType::UNARY_OPERATOR){
        ++(*it);
        StackTrace operandSearch = parseOperand(it, end, depth+1);
        if(operandSearch.success){
            std::shared_ptr<Node> ret = std::make_shared<Node>(NodeType::statement);
            ret->subtype = NodeSubType::prefix_unary;
            ret->children.push_back(std::make_shared<Node>(NodeType::_operator, std::make_shared<Token>(*prevPosition)));
            ret->children.push_back(std::move(operandSearch.node));

#ifdef PARSE_TREE_DEBUG_PRINT
            for(int i = 0; i < depth; i++){ std::cout << ".  "; }
            std::cout << "found prefix unary\n";
#endif

            return StackTrace(ret);
        }
        (*it) = prevPosition; 
        if(operandSearch.strongError){
            return operandSearch;
        }
    }

    //binary or postfix operator
    StackTrace operandSearch = parseOperand(it, end, depth+1);
    if(operandSearch.strongError){
        return operandSearch;
    }
    if(operandSearch.success){
        std::vector<Token>::iterator op1endPosition = (*it);

        //binary op
        if((*it)->type == TokenType::BINARY_OPERATOR){
            ++(*it);
            StackTrace op2Search = parseOperand(it, end, depth+1);
            if(op2Search.strongError){
                return StackTrace(ErrorType::NONE, "", {std::ref(op2Search)}, true);
            }
            if(op2Search.success){
                std::shared_ptr<Node> ret = std::make_shared<Node>(NodeType::statement);
                ret->subtype = NodeSubType::binary_op;
                ret->children.push_back(std::move(operandSearch.node));
                ret->children.push_back(std::make_shared<Node>(NodeType::_operator, std::make_shared<Token>(*op1endPosition)));
                ret->children.push_back(std::move(op2Search.node));

#ifdef PARSE_TREE_DEBUG_PRINT
                for(int i = 0; i < depth; i++){ std::cout << ".  "; }
                std::cout << "found binary op\n";
#endif

                return StackTrace(ret);
            }

            return StackTrace(ErrorType::INVALID_ARGUMENT, "bad 2nd operand of binary operator", {std::ref(op2Search)});
        }
        (*it) = op1endPosition;
        //postfix unary
        if((*it)->type == TokenType::UNARY_OPERATOR){
            std::shared_ptr<Node> ret = std::make_shared<Node>(NodeType::statement);
            ret->subtype = NodeSubType::postfix_unary;
            ret->children.push_back(std::move(operandSearch.node));
            ret->children.push_back(std::make_shared<Node>(NodeType::_operator, std::make_shared<Token>(**it)));
            ++(*it);

#ifdef PARSE_TREE_DEBUG_PRINT
            for(int i = 0; i < depth; i++){ std::cout << ".  "; }
            std::cout << "found postfix unary\n";
#endif

            return ret;
        }
    }

    StackTrace ret(ErrorType::EXPECTED_STATEMENT, "bad statement", {std::ref(operandSearch)});
    return ret;
}

StackTrace parseParentheses(std::vector<Token>::iterator* it, std::vector<Token>::iterator end, int depth){
#ifdef PARSE_TREE_DEBUG_PRINT
    for(int i = 0; i < depth; i++){ std::cout << ".  "; }
    std::cout << "parentheses | [" << (*it)->type << "] - [" << end->type << "] |\n";
#endif
    if(depth > MAX_PARSE_DEPTH){ return StackTrace(ErrorType::MAX_DEPTH, "", {}); }
    if((*it) == end){ return StackTrace(ErrorType::UNEXPECTED_EOF, "", {}); }
    std::vector<Token>::iterator startPosition = (*it);

    if((*it)->type != TokenType::OPEN_PARENTH){
        return StackTrace(ErrorType::INVALID_ARGUMENT, "No open parenth", {});
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
        return StackTrace(ErrorType::SYNTAX_ERROR, "Missing closing parenthesis", {}, true);
    }
    ++(*it);

    std::vector<Token>::iterator subIt = startPosition + 1;
    std::vector<Token>::iterator subEnd = (*it)-1;
    StackTrace argSearch = parseArgument(&subIt, subEnd, depth+1);
    if(argSearch.success){
        return argSearch;
    }

    (*it) = startPosition;
    return StackTrace(ErrorType::INVALID_ARGUMENT, "Bad inner parentheses statement", {std::ref(argSearch)}, argSearch.strongError);
}

StackTrace parseOperand(std::vector<Token>::iterator* it, std::vector<Token>::iterator end, int depth){
#ifdef PARSE_TREE_DEBUG_PRINT
    for(int i = 0; i < depth; i++){ std::cout << ".  "; }
    std::cout << "operand | [" << (*it)->type << "] - [" << end->type << "] |\n";
#endif
    if(depth > MAX_PARSE_DEPTH){ return StackTrace(ErrorType::MAX_DEPTH, "", {}); }
    if((*it) == end){ return StackTrace(ErrorType::UNEXPECTED_EOF, "", {}); }
    std::vector<Token>::iterator prevPosition = (*it);
    StackTrace valSearch = parseValue(it, end, depth+1);
    if(valSearch.strongError){
        return StackTrace(ErrorType::NONE, "", {std::ref(valSearch)}, true);
    }
    if(valSearch.success){ //value
        std::shared_ptr<Node> ret = std::make_shared<Node>(NodeType::operand);
        ret->children.push_back(std::move(valSearch.node));

#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found value\n";
#endif

        return StackTrace(ret);
    }
    (*it) = prevPosition;

    StackTrace varSearch = parseVariable(it, end, depth+1);
    if(varSearch.strongError){
        return StackTrace(ErrorType::NONE, "", {std::ref(varSearch)}, true);
    }
    if(varSearch.success){ //variable
        std::shared_ptr<Node> ret = std::make_shared<Node>(NodeType::operand);
        ret->children.push_back(std::move(varSearch.node));

#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found variable\n";
#endif

        return StackTrace(ret);
    }
    (*it) = prevPosition;

    StackTrace stateSearch = parseStatement(it, end, depth+1);
    if(stateSearch.strongError){
        return StackTrace(ErrorType::NONE, "", {std::ref(stateSearch)}, true);
    }
    if(stateSearch.success){ //statement
        std::shared_ptr<Node> ret = std::make_shared<Node>(NodeType::statement);
        ret->children.push_back(std::move(stateSearch.node));

#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found statement\n";
#endif

        return StackTrace(ret);
    }
    (*it) = prevPosition;
    return StackTrace(ErrorType::INVALID_ARGUMENT, "No valid operand found", {valSearch, varSearch, stateSearch});
}

StackTrace parseArgument(std::vector<Token>::iterator* it, std::vector<Token>::iterator end, int depth){
#ifdef PARSE_TREE_DEBUG_PRINT
    for(int i = 0; i < depth; i++){ std::cout << ".  "; }
    std::cout << "argument | [" << (*it)->type << "] - [" << end->type << "] |\n";
#endif
    if(depth > MAX_PARSE_DEPTH){ return StackTrace(ErrorType::MAX_DEPTH, "", {}); }
    if((*it) == end){ return StackTrace(ErrorType::UNEXPECTED_EOF, "", {}); }

    if(end - (*it) < 1){
        return StackTrace(ErrorType::SYNTAX_ERROR, "Argument with size <0", {}, true);
    }

    std::vector<Token>::iterator startPosition = (*it);

    //this has additional check to make sure that all tokens in parentheses are used
    StackTrace valSearch = parseValue(it, end, depth+1);
    if(valSearch.strongError){
        return StackTrace(ErrorType::NONE, "", {std::ref(valSearch)}, true);
    }
    if(valSearch.success && (*it) == end){

#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found value\n";
#endif

        return valSearch;
    }
    (*it) = startPosition;

    StackTrace varSearch = parseVariable(it, end, depth+1);
    if(varSearch.strongError){
        return StackTrace(ErrorType::NONE, "", {std::ref(varSearch)}, true);
    }
    if(varSearch.success && (*it) == end){

#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found variable\n";
#endif

        return varSearch;
    }
    (*it) = startPosition;
    
    StackTrace opSearch = parseOperand(it, end, depth+1);
    if(opSearch.strongError){
        return StackTrace(ErrorType::NONE, "", {std::ref(opSearch)}, true);
    }
    if(opSearch.success && (*it) == end){

#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found operand\n";
#endif

        return opSearch;
    }
    (*it) = startPosition;

    StackTrace stateSearch = parseStatement(it, end, depth+1);
    if(stateSearch.strongError){
        return StackTrace(ErrorType::NONE, "", {std::ref(stateSearch)}, true);
    }
    if(stateSearch.success && *it == end){

#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found statement\n";
#endif

        return stateSearch;
    }
    (*it) = startPosition;
    return StackTrace(ErrorType::INVALID_ARGUMENT, "No valid operand found",
        {valSearch, varSearch, opSearch, stateSearch});
}

StackTrace parseVariable(std::vector<Token>::iterator* it, std::vector<Token>::iterator end, int depth){
#ifdef PARSE_TREE_DEBUG_PRINT
    for(int i = 0; i < depth; i++){ std::cout << ".  "; }
    std::cout << "variable | [" << (*it)->type << "] - [" << end->type << "] |\n";
#endif
    if(depth > MAX_PARSE_DEPTH){ return StackTrace(ErrorType::MAX_DEPTH, "", {}); }
    if((*it) == end){ return StackTrace(ErrorType::UNEXPECTED_EOF, "", {}); }
    
    std::vector<Token>::iterator prevPosition = (*it);
    if((*it)->type == TokenType::IDENTIFIER){ //x
        std::shared_ptr<Node> ret = std::make_shared<Node>(NodeType::variable, std::make_shared<Token>(**it));
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
                return StackTrace(ErrorType::SYNTAX_ERROR, "Missing Closing Bracket", {}, true);
            }

            std::vector<Token>::iterator subIt = identifierEndPosition + 1;
            std::vector<Token>::iterator subEnd = *it;
            StackTrace argSearch = parseArgument(&subIt, subEnd, depth+1);
            if(argSearch.strongError){
                return StackTrace(ErrorType::NONE, "", {std::ref(argSearch)}, true);
            }
            if(argSearch.success){
#ifdef PARSE_TREE_DEBUG_PRINT
                for(int i = 0; i < depth; i++){ std::cout << ".  "; }
                std::cout << "found index\n";
#endif  
                ret->children.push_back(std::move(argSearch.node));
                ++(*it);
                identifierEndPosition = (*it);
            } else {
                return StackTrace(ErrorType::INVALID_ARGUMENT, "Malformed array index", {std::ref(argSearch)}, true); //malformed argument
            }
        }

#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found " << ((ret->children.size() > 1) ? "a[b]" : "x") << "\n";
#endif
        if(ret->children.size() > 1){
            ret->subtype = NodeSubType::array_access;
        }

        return StackTrace(ret);
    }
    (*it) = prevPosition;
    return StackTrace(ErrorType::EXPECTED_STATEMENT, "Expected Identifier", {});
}

StackTrace parseValue(std::vector<Token>::iterator* it, std::vector<Token>::iterator end, int depth){
#ifdef PARSE_TREE_DEBUG_PRINT
    for(int i = 0; i < depth; i++){ std::cout << ".  "; }
    std::cout << "value | [" << (*it)->type << "] - [" << end->type << "] |\n";
#endif
    if(depth > MAX_PARSE_DEPTH){ return StackTrace(ErrorType::MAX_DEPTH, "", {}); }
    if((*it) == end){ return StackTrace(ErrorType::UNEXPECTED_EOF, "", {}); }

    if((*it)->type == TokenType::INT_LITERAL){
        std::shared_ptr<Node> ret = std::make_shared<Node>(NodeType::value, std::make_shared<Token>(**it));
        ++(*it);

#ifdef PARSE_TREE_DEBUG_PRINT
        for(int i = 0; i < depth; i++){ std::cout << ".  "; }
        std::cout << "found int literal\n";
#endif

        return ret;
    }
    return StackTrace(ErrorType::EXPECTED_STATEMENT, "Expected INT_LITERAL", {});
}

parseTreeReturn createParseTree(std::vector<Token>& tokens){
    parseTreeReturn ret;
    std::vector<Token>::iterator lineStart = tokens.begin();
    std::vector<Token>::iterator lineEnd;
    for(std::vector<Token>::iterator it = tokens.begin(); it != tokens.end(); ++it){
        if(it->type == TokenType::SEMI){
            lineEnd = it;

            ret.traces.push_back(parseStatement(&lineStart, lineEnd, 0));
            if(!ret.traces.back().success){
                ret.success = false;
            }

            lineStart = it+1;
        }
    }
    return ret;
}

#endif