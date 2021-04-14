/*
 * File: Parser.cpp
 * Ziqi Gao 118010077
 * -----------------
 * This file contains the implementation of Parser.
 * It consists of three main parts:
 * (1) Some Tools
 * (2) Parser and AST Node Constructor
 * (3) AST Node Traversaler
 *-----------------------------------------------
 */

#include "Parser.h"
#include "Global.h"
#include "env.h"
#include <QDebug>
/* Global Variable Initialization */
std::shared_ptr<Program> file1;




Parser::Parser() {
    lookahead = nullptr;
    lineCounter = 0;
    iter_line = allToken.begin();
    iter_token = (*iter_line).begin();
    global_table_ptr = new Env;
    current_table_ptr = global_table_ptr;

}

void Parser::syntactic_analysis() {
    Lookahead();
    program_parser();
}


/* Part1: Tools */

/*
 * Implementation Notes
 * --------------------------------
 * Those two methods return the name/value of
 * the being read token.
 * We need to cast the pointer of a base class to a
 * pointer pointing to derived class.
 * -----------------------------------------------
 */
std::string Parser::get_name() {
    auto ptr = std::static_pointer_cast<Word>(lookahead);
    std::string name = ptr->lexemes;
    return name;
}

int Parser::get_value() {
    auto ptr = std::static_pointer_cast<Num>(lookahead);
    int value = ptr->value;
    return value;
}


/*
 * Implementation Notes
 * ----------------------------------
 * Those two methods get one more token
 * from the 2D tokens vector.
 */
void Parser::Lookahead() {
    lookahead = get_Token();
}
std::shared_ptr<Token> Parser::get_Token() {
    //Case 1: Continue to read tokens in this line.
    if (iter_token != (*iter_line).end()) {
        auto result(*iter_token);
        ++iter_token;
        return result;
    }
        //Case 2: This line's tokens have all already been read or the line is empty.
        // Go to search another line having tokens.
        // In this situation, you can always get the first token in the line.
    else {
        while (iter_line != allToken.end()) {
            ++lineCounter;
            ++iter_line;
            iter_token = (*iter_line).begin();
            if (iter_token == (*iter_line).end()) continue;
            else {
                auto result(*iter_token);
                ++iter_token;
                return result;
            }

        }
    }
    return nullptr;
}



/*
 * Implementation Notes:
 * ----------------------------
 * Those three methods will check whether the
 * being read token's tag is correct.
 * match and match_Go will throw syntax error
 * if the check fails.
 * Check will just do the match without throwing
 * error.
 * ALso, match_Go will keep read one more token. If the
 * test passes.
 * ---------------------------------------------------
 */
bool Parser::match(TYPE tag) {
    if (lookahead->tag == tag) {
        return true;
    } else {
        qDebug() << "Syntax Error at line: " << lineCounter;
        throw MyException("Syntax Error");
    }
}

bool Parser::check(TYPE tag) {
    return (lookahead->tag == tag);
}

bool Parser::match_Go(TYPE tag) {
    if (lookahead->tag == tag) {
        Lookahead();
        return true;
    } else {
        qDebug() << "Syntax Error at line: " << lineCounter;
        throw MyException("Syntax Error");
        return false;
    }
}

/*
 * Part 2: Syntactic Analysis (Parser) and AST Generator
 * ------------------------------------------------------
 * Those methods are designed according to the context-free
 * grammar (will be introduced in detail)
 * Those methods are also like a tree that will be called in
 * order from the "root" of those parser methods.
 * They will throw error if the grammar in code is wrong.
 *
 * Also, "if" expression has to follow a block in a pair of curly braces,
 * which is different from C/C++ language in order to simplify the work.
 *
 * And while expression should only hold one condition statement
 * inside a pair of braces. Otherwise, it will throw syntax error.
 *
 * Since we cannot call function and it is hard to translate, we do
 * not support function parser. This program will only give a simple
 * syntax check to chekc whether a function have return type. The function
 * given by the user cannot have arguments list otherwise it will throw
 * error.
 *
 * No symbol table is built so this program cannot deal with the scope
 * of variable. All variables are considered as global variables.
 * ----------------------------------------------------------------
 * Since my part of code cannot be integrated in the GUI perfectly,
 * the detail of the syntax error can only be shown in the terminal
 * instead of in GUI.
 */



void Parser::program_parser() {
    file1 = std::make_shared<Program>(PROGRAM, "c.c");
    //Parse the function(s) and link them to file1(Root of the AST).
    while (!check(END)) {
        auto record = lookahead;
        auto line_record = iter_line;
        auto token_record = iter_token;
        //Rollback, if it is not a global variable.
        if(!global_parser()){
            lookahead = record;
            iter_token = token_record;
        }
        while(true){
            record = lookahead;
            line_record = iter_line;
            token_record = iter_token;
            if(!global_parser()){
                lookahead = record;
                iter_token = token_record;
                break;
            }
            else continue;
        }

        //Add all functions node into the file (program) node
        file1->blocks.push_back(function_parser());
    }
    //Check whether a main fucntion is detected.
    if (func_table.find("main") == func_table.end()) {
        qDebug() << "Cannot find main function!";
        throw MyException("Syntax Error");
        //exit(EXIT_FAILURE);
    }
    //Check the return type of main()
    else {
        if (func_table["main"] != INT) {
            qDebug() << "::main must return INT";
            throw MyException("Syntax Error");
            //exit(EXIT_FAILURE);
        }
    }
}

bool Parser::global_parser(){
    TYPE variable_type;
    auto tag = lookahead;
    if (check(INT)) variable_type = INT;
    else if (check(CHAR)) variable_type = CHAR;
    else if (check(FLOAT)) variable_type = FLOAT;
    else if (check(VOID)) variable_type = VOID;
    else {
        std::cerr << "Syntax Error (unknown variable type)"
                  << "at line: " << lineCounter << std::endl;
        exit(EXIT_FAILURE);
    }
    Lookahead();
    match(ID);
    std::string name = get_name();
    Lookahead();
    if(!check(LB)){
        //Function override???
        if(global_table_ptr->table.find(name) != global_table_ptr->table.end()){std::cerr << "XXX has been defined" << std::endl;}
        if(check(ASSIGNMENT)){
            Lookahead();
            if(check(ID)||check(NUM)){
                auto temp = factor_parser();
                std::shared_ptr<Number> value_ptr = std::static_pointer_cast<Number>(temp);
                global_table_ptr->table.insert({name, value_ptr->value});
            }
            else{
                qDebug() << "Syntax Error. Cannot Initialize Global variable: " << QString::fromStdString(name);
                throw MyException("Syntax Error");
            }
            //Skip ; at the end of initialization.
            Lookahead();
            return true;
        }
        if(check(SEMICOLON)){
            qDebug()<< "Syntax Error: Global Variable has to be initialized";
            throw MyException("Syntax Error");
        }
    }

    return false;
}

std::shared_ptr<Compound> Parser::function_parser() {
    std::shared_ptr<Compound> funcNode;
    //Return Type Checker
    function_declaration_parser(funcNode);
    //Implementation of Functions
    compound_parser(funcNode);
    return funcNode;
}

void Parser::function_declaration_parser(std::shared_ptr<Compound> &funcNode) {
    TYPE return_type;
    //Check whether a function has return type.
    if (check(INT)) return_type = INT;
    else if (check(CHAR)) return_type = CHAR;
    else if (check(FLOAT)) return_type = FLOAT;
    else if (check(VOID)) return_type = VOID;
    else {
        qDebug()<< "Syntax Error (Need a return type of function)"
                  << "at line: " << lineCounter;
        throw MyException("Syntax Error");
        //exit(EXIT_FAILURE);
    }
    /* Need a Symbols Table to record the function's name,
     * return type and argument list. There is no time for me
     * to do that.
    ----------------------------------------------------- */
    Lookahead();
    match(ID);
    //Get the name of the function and build the node for the function.
    auto ptr = std::static_pointer_cast<Word>(lookahead);
    std::string name = ptr->lexemes;
    funcNode = std::make_shared<Compound>(COMPOUND, name);
    func_table.insert({{name, return_type}});
    Lookahead();
    //Ignore arguments of functions.
    //(As a result, user cannot define a function with arguments list )
    match_Go(LB);
    match_Go(RB);


}

void Parser::compound_parser(std::shared_ptr<Compound> &func) {
    match_Go(LCB);
    Env *env_table_ptr = new Env(*current_table_ptr);
    current_table_ptr = env_table_ptr;
    //Check "{"
    while (lookahead->tag != RCB) {
        if (lookahead->tag == END) {
            qDebug() << "expected } at end of input at line: " << lineCounter;
            //exit(EXIT_FAILURE);
            throw MyException("Syntax Error");
        }
        //Check statements one by one.
        statement_parser(func);
    }
    current_table_ptr = current_table_ptr->prev;
    delete env_table_ptr;
    //Check "}"
    match_Go(RCB);

}

void Parser::statement_parser(std::shared_ptr<Compound> &func) {
    using std::shared_ptr;
    using std::static_pointer_cast;
    using std::make_shared;
    using std::string;
    //There are 7 possible situations if the grammar is correct.
    //For loop and function calling are not included.
    auto tag = lookahead->tag;
    switch (tag) {
        //if(conditon){Statements}
        case IF: {
            Lookahead();
            match_Go(LB);
            shared_ptr<Operator> ifNode = make_shared<Operator>(IF);
            ifNode->left = bool_parser();
            match_Go(RB);
            if (check(LCB)) {
                //Set a new compound node
                shared_ptr<Compound> DO = make_shared<Compound>(COMPOUND);
                compound_parser(DO);
                ifNode->right = DO;
            } else {
                qDebug() << "Syntax Error at line: " << lineCounter;
                throw MyException("A block should follow the condition.");
                ////exit(EXIT_FAILURE);
                return;
            }
            func->Children.push_back(ifNode);
            break;
        }
        //Printf can only hold a number/variable to simplify the work.
        //printf(a);
        case PRINTF: {
            Lookahead();
            match_Go(LB);
            shared_ptr<Operator> print = make_shared<Operator>(PRINTF);
            if (check(NUM)) {
                int value = get_value();
                shared_ptr<Number> constant = make_shared<Number>(NUM, value);
                print->right = constant;
            } else if (check(ID)) {
                string name = get_name();
                if (!current_table_ptr->get_id(lookahead)) {
                    qDebug() << "Syntax error at line: " << lineCounter;
                    //exit(EXIT_FAILURE);
                    throw MyException("Undefined Identifier");
                }
                int value = get_value();
                shared_ptr<Variable> id = make_shared<Variable>(ID, name, value);
                print->right = id;
            } else {
                qDebug() << "Syntax Error at line: " << lineCounter;
                throw MyException("Can only print out a number or a variable's value");
            }
            Lookahead();
            match_Go(RB);
            func->Children.push_back(print);
            break;
        }
        //while(condition){statements}
        case WHILE: {
            Lookahead();
            match_Go(LB);
            shared_ptr<Operator> whileNode = make_shared<Operator>(WHILE);
            whileNode->left = bool_parser();
            match_Go(RB);
            if (check(LCB)) {
                //Set a new compound node
                shared_ptr<Compound> DO = make_shared<Compound>(COMPOUND);
                compound_parser(DO);
                whileNode->right = DO;
            } else {
                qDebug() << "A block should follow the condition."
                          << "Syntax Error at line: " << lineCounter;
                //exit(EXIT_FAILURE);
                throw MyException("Syntax Error: A block should follow the condition");
                return;
            }
            func->Children.push_back(whileNode);
            break;
        }

        // It may be a declaration or assignment.
        // Assignment ID (should be defined before) = num/variable +-*/ variable
        // variable = expression + expression
        case ID: {
            shared_ptr<Operator> assignNode;
            string id = get_name();
            shared_ptr<Variable> vptr;
            if(current_table_ptr->get_id(lookahead)){
                vptr = make_shared<Variable>(Variable(ID, id));
            }
            else{
                qDebug() << "Undefined Identifier: " << QString::fromStdString(id)
                          << " at line: " << lineCounter;
                throw MyException("Syntax Error: Undefined Identifier");
            }
            Lookahead();
            match_Go(ASSIGNMENT);
            assignNode = make_shared<Operator>(ASSIGNMENT);
            assignNode->left = vptr;
            assignNode->right = expression_parser();
            //Add it to the Function (compound) vector.
            func->Children.push_back(assignNode);
            break;
        }
            //int ID = ...;
            // or int ID;
            //Declaration | Assignment
        case INT: {
            std::string id_name;
            Lookahead();
            match(ID);
            id_name = get_name();
            //Check redeclaration:
            if (current_table_ptr->get_id(lookahead)) {
                qDebug() << QString::fromStdString(id_name) << " has already been declared!";
                //exit(EXIT_FAILURE);
                throw MyException("Syntax Error: Identifier has already been declared!");
                return;
            }
            Lookahead();
            //If we can find a '=', initialize ID
            if (check(ASSIGNMENT)) {
                Lookahead();
                auto assignNode = make_shared<Operator>(ASSIGNMENT);;
                auto vptr = make_shared<Variable>(Variable(ID, id_name));
                assignNode->left = vptr;
                assignNode->right = expression_parser();
                //Add this id into id_table
                current_table_ptr->table.insert({id_name, 0});
                func->Children.push_back(assignNode);
            }
            else if (check(SEMICOLON)){
                auto assignNode = make_shared<Operator>(ASSIGNMENT);;
                auto vptr = make_shared<Variable>(Variable(ID, id_name));
                assignNode->left = vptr;
                assignNode -> right = make_shared<Number>(Number(NUM, 0));
                func->Children.push_back(assignNode);
                current_table_ptr->table.insert({id_name, 0});
                Lookahead();
                }
            else {
                qDebug() << "Expect a ;  at line" << lineCounter - 1;
                //exit(EXIT_FAILURE);
                throw MyException("Expect a ;  at line");
                return;
            }
            break;

        }
        case SEMICOLON:
            Lookahead();
            break;
            //An constant number cannot be a left value.
        case NUM:
            qDebug() << "A constant number cannot be a left value"
                      << "Syntax Error at line: " << lineCounter;
            throw MyException("Syntax Error: Wrong Left Value");
            //exit(EXIT_FAILURE);
            return;
            //Other cases are considered as syntax error.
        default:
            qDebug() << "Syntax Error (Wrong Statement) at Line: " << lineCounter;
            //exit(EXIT_FAILURE);
            throw MyException("Syntax Error: Wrong Statement");
            return;
    }
}
//This method will be called in condition flow.
std::shared_ptr<Statement> Parser::bool_parser() {
    using std::shared_ptr;
    using std::make_shared;
    shared_ptr<Statement> temp = expression_parser();
    //Nodes can be an completed expression
    switch (lookahead->tag) {
        // <
        case LT: {
            shared_ptr<Operator> result = make_shared<Operator>(LT);
            Lookahead();
            result->left = temp;
            result->right = expression_parser();
            return result;
        }
            // >
        case GT: {
            shared_ptr<Operator> result = make_shared<Operator>(GT);
            Lookahead();
            result->left = temp;
            result->right = expression_parser();
            return result;
        }
            // !=
        case NEQ: {
            shared_ptr<Operator> result = make_shared<Operator>(NEQ);
            Lookahead();
            result->left = temp;
            result->right = expression_parser();
            return result;
        }
            // ==
        case EQ: {
            shared_ptr<Operator> result = make_shared<Operator>(EQ);
            Lookahead();
            result->left = temp;
            result->right = expression_parser();
            return result;
        }
            // >=
        case GE: {
            shared_ptr<Operator> result = make_shared<Operator>(GE);
            Lookahead();
            result->left = temp;
            result->right = expression_parser();
            return result;
        }
            // <=
        case LE: {
            shared_ptr<Operator> result = make_shared<Operator>(LE);
            Lookahead();
            result->left = temp;
            result->right = expression_parser();
            return result;
        }
        default: {
            qDebug() << "Syntax error at line:  " << lineCounter;
            throw MyException("Syntax Error");
            return nullptr;
        }
    }
    return temp;
}

/*
 * Implementation Notes:
 * -------------------------------
 * Since plus and minus have lower priority than multiply, divide
 * and (...) they will be added into the AST earlier.
 * Nodes at left will be calculated before the right nodes.
 */


std::shared_ptr<Statement> Parser::expression_parser() {
    using std::shared_ptr;
    using std::make_shared;
    //Check operators with higher priority
    shared_ptr<Statement> temp = term_parser();
    if (lookahead->tag == PLUS) {
        Lookahead();
        shared_ptr<Operator> result = make_shared<Operator>(PLUS);
        result->left = temp;
        result->right = expression_parser();
        return result;
    }
    if (lookahead->tag == MINUS) {
        Lookahead();
        shared_ptr<Operator> result = make_shared<Operator>(MINUS);
        result->left = temp;
        result->right = expression_parser();
        return result;
    }
    return temp;
}

std::shared_ptr<Statement> Parser::term_parser() {
    using std::shared_ptr;
    using std::make_shared;
    auto temp = factor_parser();
    if (lookahead->tag == MUL) {
        Lookahead();
        shared_ptr<Operator> result = make_shared<Operator>(MUL);
        result->left = temp;
        result->right = term_parser();
        return result;
    }
    if (lookahead->tag == DIV) {
        Lookahead();
        shared_ptr<Operator> result = make_shared<Operator>(DIV);
        result->left = temp;
        result->right = term_parser();
        return result;
    }
    return temp;
}

std::shared_ptr<Statement> Parser::factor_parser() {
    using std::make_shared;
    std::shared_ptr<Statement> temp = nullptr;
    int constant;
    switch (lookahead->tag) {
        //Negative number Case
        case MINUS:
            Lookahead();
            if(check(ID)){
                std::string name = get_name();
                if (!current_table_ptr->get_id(lookahead)) {
                    qDebug() << "Undefined Identifier " << QString::fromStdString(name)
                              << " at line: "
                              << lineCounter;
                    throw MyException("Syntax Error");
                } else {
                    temp = make_shared<Variable>(ID, name, -1);
                    Lookahead();
                    return temp;
                }
            }
            else if(check(NUM)){
                constant = -(get_value());
                temp = make_shared<Number>(NUM, constant);
                Lookahead();
                return temp;
            }
            else{
                qDebug() << "Syntax Error at line:" << lineCounter;
                throw MyException("Syntax Error");
                return nullptr;
            }
        case LB: //Case "(expr)":
            Lookahead();
            temp = expression_parser();
            match_Go(RB);
            return temp;
        // A positive number (without - in front of the number)
        case NUM:
            //Get the number's value in the token:
            constant = get_value();
            temp = make_shared<Number>(NUM, constant);
            Lookahead();
            return temp;
       // Case Variable
        case ID: {
            std::string name = get_name();
            if (!current_table_ptr->get_id(lookahead)) {
                qDebug() << "Undefined Identifier " << QString::fromStdString(name)
                          << " at line: "
                          << lineCounter;
                throw MyException("Syntax Error");
            } else {
                temp = make_shared<Variable>(ID, name);
                Lookahead();
                return temp;
            }
        }

        default:
           qDebug() << "Syntax Error at line:" << lineCounter;
           throw MyException("Syntax Error");
           return nullptr;
    }
}

/*
 * Part 3: Those methods will traverse the Abstract Syntax
 * Tree in post order. Post order can correctly show the
 * Mathematical calculation order correctly.
 * ---------------------------------------------------
 * Also, since my program cannot combine with the GUI perfectly,
 * I have to print the result in the terminal and save my result
 * into a file that will be read by GUI and show the result on it.
 */

// This method will print out the name of each function.
void Parser::AST_traversal() {
    using std::endl;
    for (auto compound_ptr : file1->blocks) {
        ordinal = 1;
        std::cout << "Function: " << compound_ptr->func_name << ": " << endl;
        result_file << "Function: " << compound_ptr->func_name << ": " << endl;
        COMPOUND_traversal(compound_ptr);
    }
}

/*
 * Implementation Notes:
 * ------------------------------
 * This method will read all the statements in
 * a block(function or block of while/if/for loop).
 * This function will call the "lower case" traversaler
 * to continue to print the result. In other words,
 * it will traverse those subtrees.
 */
void Parser::COMPOUND_traversal(std::shared_ptr<Compound> &compound) {
    using std::endl;
    using std::shared_ptr;
    using std::static_pointer_cast;
    for (auto statement_ptr:compound->Children) {
        std::cout << "Statement " << ordinal << ": ";
        result_file << "Statement " << ordinal << ": ";
        auto temp = statement_ptr;
        auto opNode = std::static_pointer_cast<Operator>(temp);
        /* There are four kinds of Statements:
         * Assignment e.g. a = b + c;
         * PRINTF: prinf(1);
         * WHILE: while(a<b){...};
         * IF: if(a<b){...};
         */
        switch (opNode->type) {
            case ASSIGNMENT:
                ASSIGNMENT_traversal(opNode);
                break;
            case PRINTF:
                PRINTF_traversal(opNode);
                break;
            case WHILE:
                WHILE_traversal(opNode);
                break;
            case IF:
                IF_traversal(opNode);
                break;
        }
        ++ordinal;
    }
    //Print the root of the sub-tree.
    std::cout << endl;
    result_file << endl;
}
/* Implementation Notes:
 *------------------------------------------------------------
 * Since my program cannot combine with the GUI perfectly,
 * I have to print the result in the terminal and save my result
 * into a file that will be read by GUI and show the result on it.
 */
void Parser::IF_traversal(std::shared_ptr<Operator> & ifNode) {
    using std::endl;
    using std::shared_ptr;
    using std::static_pointer_cast;
    std::cout << endl << "if statement: " << endl;
    std::cout << "Condition: ";
    result_file << endl << "if statement: " << endl;
    result_file << "Condition: ";
    auto left0 = ifNode->left;
    auto right0 = ifNode->right;
    auto left = static_pointer_cast<Operator>(left0);
    auto right = static_pointer_cast<Compound>(right0);
    algo_traversal(left);
    std::cout << endl;
    std::cout << "Execution: " << endl;
    result_file << endl;
    result_file << "Execution: " << endl;
    COMPOUND_traversal(right);
}

void Parser::WHILE_traversal(std::shared_ptr<Operator> &whileNode)  {
    using std::endl;
    using std::shared_ptr;
    using std::static_pointer_cast;
    std::cout << endl << "while statement: " << endl;
    std::cout << "Condition: ";
    result_file << endl << "while statement: " << endl;
    result_file << "Condition: ";
    auto left0 = whileNode->left;
    auto right0 = whileNode->right;
    auto left = static_pointer_cast<Operator>(left0);
    auto right = static_pointer_cast<Compound>(right0);
    algo_traversal(left);
    std::cout << endl;
    result_file << endl;
    //Right hand side (A block):
    std::cout << "Execution: " << endl;
    result_file << "Execution: " << endl;
    COMPOUND_traversal(right);
}


void Parser::PRINTF_traversal(std::shared_ptr<Operator> &printNode){
    using std::endl;
    std::cout << "printf: ";
    result_file << "printf: ";
    auto temp = printNode->right;
    if (temp->type == NUM) {
        auto right = std::static_pointer_cast<Number>(temp);
        std::cout << right->value << endl;
        result_file << right->value << endl;
    } else {
        auto right = std::static_pointer_cast<Variable>(temp);
        std::cout << right->name << endl;
        result_file << right->name << endl;
    }
}

void Parser::ASSIGNMENT_traversal(std::shared_ptr<Operator> &assignNode){
    using std::endl;
    auto left0 = assignNode->left;
    auto right0 = assignNode->right;
    //Left Node -> Right Node -> Root.
    auto left = std::static_pointer_cast<Variable>(left0);
    std::cout << left->name << " ";
    result_file << left->name << " ";
    //The right node may be a number in most of the cases.
    if (right0->type == NUM) {
        auto right = std::static_pointer_cast<Number>(right0);
        std::cout << right->value;
        result_file << right->value;
    }
    else if(right0->type == ID){
        auto right = std::static_pointer_cast<Variable>(right0);
        std::cout << right->name;
        result_file << right->name;
    }
    //Else, the expression is more complex in a = b+c*d mode.
    //Just link the operator to the right node.
    else {
        auto temp = right0;
        auto opNode = std::static_pointer_cast<Operator>(temp);
        //Traverse back to check the expression.
        algo_traversal(opNode);
    }
    std::cout << " = " << endl;
    result_file << " = " << endl;
}

void Parser::algo_traversal(std::shared_ptr<Operator> &opNode){
    using std::endl;
    auto left0 = opNode->left;
    auto right0 = opNode->right;
    // Left node of an operator except "=" can be a number/variable
    // or another expression.
    if (left0->type == NUM) {
        auto left = std::static_pointer_cast<Number>(left0);
        std::cout << left->value << " ";
        result_file << left->value << " ";
    } else if (left0->type == ID) {
        auto left = std::static_pointer_cast<Variable>(left0);
        std::cout << left->name << " ";
        result_file << left->name << " ";
    }
        // Left node can also be an expression when () is used.
    else {
        auto left = std::static_pointer_cast<Operator>(left0);
        algo_traversal(left);
    }
    //Right Node Part:
    if (right0->type == NUM) {
        auto right = std::static_pointer_cast<Number>(right0);
        result_file << right->value << " ";
        std::cout << right->value << " ";
    }
        //Or it may be an identifier.
    else if (right0->type == ID) {
        auto right = std::static_pointer_cast<Variable>(right0);
        std::cout << right->name << " ";
        result_file << right->name << " ";
    }
        //Else, it is also an operator
    else {
        auto right = std::static_pointer_cast<Operator>(right0);
        algo_traversal(right);
    }

    switch (opNode->type) {
        case PLUS:
            std::cout << " + ";
            result_file << " + ";
            break;
        case MINUS:
            std::cout << " - ";
            result_file << " - ";
            break;
        case MUL:
            std::cout << " * ";
            result_file << " * ";

            break;
        case DIV:
            std::cout << " / ";
            result_file << " / ";
            break;
        case LT:
            std::cout << " < ";
            result_file << " < ";
            break;
        case GT:
            std::cout << " > ";
            result_file << " > ";
            break;
        case LE:
            std::cout << " <= ";
            result_file << " <= ";
            break;
        case GE:
            std::cout << " >= ";
            result_file << " >= ";
            break;
        case NEQ:
            std::cout << " != ";
            result_file << " != ";
            break;
        case EQ:
            std::cout << " == ";
            result_file << " == ";
            break;
    }
}

