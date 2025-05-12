#define DONTRUNTHIS
#ifndef DONTRUNTHIS

#ifndef PARSETREE_CPP
#define PARSETREE_CPP

#include <vector>
#include <iostream>
#include <map>

#include "token.h"
//#include "syntaxDefinitions.h"
#include "parseTree.h"


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


#define MAX_DEPTH 10

std::ostream& operator<<(std::ostream& out, NodeType t){ return out << NodeTypeStrings[(int)t]; }

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
        std::cout << "-";
    }
    std::cout << "(" << type << ")";
    if(token != nullptr){
        std::cout << " : " << token->type;
    }
    std::cout << "\n";
    for(int i = 0; i < children.size(); i++){
        children.at(i)->print(depth+1);
    }
}

//gets passed a stack which represents a target's possible definition, 
//tries to compute a valid tree from that
//true if success, false if failed
bool parseTreeBacktracking(std::vector<Token>::iterator* it, std::vector<Token>::iterator end, std::vector<Node*>* stack, Node* root, int depth=0){
    for(int i = 0; i < depth; i++){
        std::cout << "\t";
    }
    std::cout << root->type << "\n";
 
    if(depth > MAX_DEPTH){
        return false;
    }

    std::vector<std::vector<NodeType>>* definitions; //syntax definitions
    std::vector<Node*> argStack; // stack to pass to further calls 

    while(!stack->empty()){
        while(*it != end && (*it)->type == TokenType::NEWLINE){
            it++;
        }

        //ran out of tokens before eof or semicolon
        if(*it == end || (*it)->type == TokenType::SEMI){
            //std::cerr << "Error, expected: "; 
            //std::cerr << target->type << "at eof\n";
            return false;
        }

        Node* target = stack->back();
        stack->pop_back();

        for(int i = 0; i < depth; i++){
            std::cout << "\t";
        }
        std::cout << ": " << target->type << "\n";

        //special case tokens
        if(hasSimpleSyntaxDefinition.find(target->type) != hasSimpleSyntaxDefinition.end()){
            std::cout << "$\n";
            bool notFound = true;
            switch(target->type){
                case NodeType::value:
                    if((*it)->type == TokenType::INT_LITERAL){
                        target->token = &(**it); 
                        root->children.push_back(target);
                        (*it)++; notFound = false;
                    }
                    break;
                case NodeType::open_parenth:
                    if((*it)->type == TokenType::OPEN_PARENTH){
                        target->token = &(**it); 
                        root->children.push_back(target);
                        (*it)++; notFound = false;
                    }
                    break;
                case NodeType::close_parenth:
                    if((*it)->type == TokenType::CLOSE_PARENTH){
                        target->token = &(**it); 
                        root->children.push_back(target);
                        (*it)++; notFound = false;
                    }
                    break;
                case NodeType::open_bracket:
                    if((*it)->type == TokenType::OPEN_BRACKET){
                        target->token = &(**it); 
                        root->children.push_back(target);
                        (*it)++; notFound = false;
                    }
                    break;
                case NodeType::close_bracket:
                    if((*it)->type == TokenType::CLOSE_BRACKET){
                        target->token = &(**it); 
                        root->children.push_back(target);
                        (*it)++; notFound = false;
                    }
                    break;
                case NodeType::unary_operator:
                    if((*it)->type == TokenType::UNARY_OPERATOR){
                        target->token = &(**it); 
                        root->children.push_back(target);
                        (*it)++; notFound = false;
                    }
                    break;
                case NodeType::binary_operator:
                    if((*it)->type == TokenType::BINARY_OPERATOR){
                        target->token = &(**it); 
                        root->children.push_back(target);
                        (*it)++; notFound = false;
                    }
                    break;
                case NodeType::identifier:
                    if((*it)->type == TokenType::IDENTIFIER){
                        target->token = &(**it); 
                        root->children.push_back(target);
                        (*it)++; notFound = false;
                    }
                    break;
                default:
                    std::cerr << "Unkown syntax definition of " << target->type << "\n";
                    return false;
            }
            if(notFound){
                return false;
            }
            continue;
        }

        if(target->type == NodeType::statement){
            //handle function call syntax
            //std::cerr << "function call not implemented yet\n";
            //return false;
        }


        definitions = &syntaxDefinition.at(target->type);
        for(std::vector<std::vector<NodeType>>::iterator d = definitions->begin(); d != definitions->end(); ++d){
            //create new stack with definition d of target
            for(std::vector<NodeType>::iterator obj = d->begin(); obj != d->end(); obj++){
                argStack.push_back(new Node(*obj));
            }

            std::vector<Token>::iterator itCopy = (*it);

            //search recursively for matching definition d
            bool searchRet = parseTreeBacktracking(it, end, &argStack, target, depth+1);

            if(searchRet == false){ //search failed
                *it = itCopy;
                continue;
                //argStack empty
            }
            root->children.push_back(target);
            break;
        }
    }

    return true;
}


Node* createParseTree(std::vector<Token>& tokens){
    std::vector<Node*> stack;
    stack.push_back(new Node(NodeType::statement));
    Node* head = stack.at(0);
    int linenumber = 1;


    std::cout << "~~~~~~\n";
    std::cout << hasSimpleSyntaxDefinition.find(NodeType::value)->second << "\n";
    std::cout << syntaxDefinition.find(NodeType::operand)->second.size();
    std::cout << "\n~~~~~~\n";

    std::vector<Token>::iterator it = tokens.begin();
    
    if(parseTreeBacktracking(&it, tokens.end(), &stack, head)){
        return head;
    } else {
        delete head;
        return nullptr;
    }
    
}

/*
Psuedocode

start with an empty stack
push <statement> to stack
tree head = <statement>
cur = head


vector<Node*> recursiveSearch(iterator, stack){
    root = nullptr
    while(stack not empty):
        target = stack.pop()
        if root == nullptr:
            root = target
        for d in definitions of target:
            create stack s1 with members of d
            blocks = recursiveSearch(it, s1)
            if search succeeds:
                target.children = blocks
                break
        if no valid definitions found:
            throw error
    
}

x++

stack: 

statement
    postfix_unary_op
        operand
            identifier=x
        unary_operator=++
    

parse x + y:
<stack> root

<statement> ROOT
    function syntax //fails
    < (, statement, ) > statement
        -> fails on (
    < unary, operand > statement
        -> fails on unary
    < operand, unary > statement
        < variable > operand
            < identifier > variable
                succeeds on x
                return identifier(x)
            < > variable->identifier
        < unary > operand->variable->identifier
        < > finds unary = +, operand->(variable->identifier=x)
        < > operand->(variable->identifier=x, unary='+')
        succeeds with ROOT->statement->operand->(variable=x, unary='+')
        fails because not all significant tokens used (can ignore ;\n)

*/


#endif

#endif