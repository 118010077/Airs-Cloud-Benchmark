/*
 * File Name: Program.h
 * ---------------------------
 * This file shows the design of
 * the nodes in Abstract Syntax Tree(AST)
 * I will give you a clear graph to show
 * the relationship between those classes.
 * ---------------------------------------
 */

#ifndef SYNTACTIC_PROGRAM_H
#define SYNTACTIC_PROGRAM_H

#include <vector>
#include <memory>
#include "Token.h"
#include "Lexer.h"

//Class Declaration
class Compound;

class Number;

class Statement;

class Operator;
//------------------------


//Statement is the basic node in the AST.
class Statement {
public:
    Statement(enum TYPE symbolType) {
        type = symbolType;
    }

    enum TYPE type;
};

//Divide the program into different blocks. Each block is a function (compound).
class Program : public Statement {
public:
    Program(enum TYPE symbolType, std::string name = "c.c") : Statement(symbolType) { file_name = name; }

    std::vector<std::shared_ptr<Compound>> blocks;
    std::string file_name;
};


// I combine function and compound(block) together. The only difference between those two
// is that function has a name. To simplify the program, I do not make a distinction here.
class Compound : public Statement {
public:
    Compound(enum TYPE symbolType, std::string name = "NULL")
            : Statement(symbolType) { func_name = name; }

    std::vector<std::shared_ptr<Operator>> Children;
    std::string func_name;
};


//Operator is the subclass of statement. It has two pointers that can link the
//variables/constants(Numbers)/operator(+-*/)
class Operator : public Statement {
public:
    Operator(enum TYPE symbolType, std::shared_ptr<Statement> leftptr = nullptr,
             std::shared_ptr<Statement> rightptr = nullptr) : Statement(symbolType) {
        left = leftptr;
        right = rightptr;
    }

    std::shared_ptr<Statement> left;
    std::shared_ptr<Statement> right;
};


//Number is also the subclass of statement. It has an integer value.
class Number : public Statement {
public:
    Number(enum TYPE symbolType, int IDvalue) : Statement(symbolType) {
        value = IDvalue;
    }

    int value;
};

//Variable is a subclass of Number. It has a name and an integer value that is
//set to be 0 by default.
class Variable : public Number {
public:
    Variable(enum TYPE symbolType, std::string IDname, int value = 0) : Number(symbolType, value) {
        name = IDname;
        IDvalue = value;
    }

    std::string name;
    int IDvalue;

};

#endif //SYNTACTIC_PROGRAM_H
