/* File:Parser.h
 * Created by Andy_Gao on 5/26/2020.
 * -----------------------------------------------
 * This file define the class:Parser
 * This class will read token from the
 * lexer and generate the Abstract Syntax Tree.
 * Also, it will also conduct a grammar check to
 * see whether there it is any syntax error in the
 * code.
 */

#ifndef SYNTACTIC_PARSER_H
#define SYNTACTIC_PARSER_H

#include "Token.h"
#include "Lexer.h"
#include "Global.h"
#include "env.h"
#include "unordered_set"
#include "unordered_map"
#include <stdexcept>
#include <memory>
#include <vector>

class Parser {
public:
    Parser();

    ~Parser() {delete global_table_ptr; }
    //The following two iterator will keep reading token from that 2D array --- allToken.
    std::vector<std::vector<std::shared_ptr<Token>>>::iterator iter_line; //This iterator search through the line.
    std::vector<std::shared_ptr<Token>>::iterator iter_token; //This iterator search through tokens for every line.
    //This method will conduct syntacitc analysis.
    void syntactic_analysis();

    //This method will print out the AST.
    void AST_traversal();

    //This method can get a token from the 2D array ---allToken
    std::shared_ptr<Token> get_Token();

private:
    //---------------------------------Variables Declaration---------------------------------
    //id_table saves the variables that are already been defined or declared.
    std::unordered_set<std::string> id_table;
    Env *global_table_ptr;
    Env *current_table_ptr;
    //func_table saves the function name and its return type.
    std::unordered_map<std::string, TYPE> func_table;
    //lineCounter record the location of the being read token(s) to locate syntax error.
    int lineCounter;
    //This variable is used to record the location of statement.
    int ordinal;
    //lookahead read a token from the 2D Tokens vector.
    std::shared_ptr<Token> lookahead;
    //----------------------------------------------------------------------------------------
    //---------------------------------Method Declaration-------------------------------------
    /*
     * Method:match(), match_Go()
     * Usage: match(TYPE), match_Go(TYPE)
     * ----------------------------------
     * Those two functions are almost the same
     * that will check whether the lookahead token
     * can match the argument. And return the bool value
     * of the result.
     * match_Go() will read the next token by the way,
     * but match() won't.
     * They will throw "Syntax Error" if the match fails.
     * ----------------------------------------------
     */
    bool match(TYPE);

    bool match_Go(TYPE);

    /*
     * Method: check()
     * Usage: check(TYPE)
     * ----------------------------------------
     * This method will also check whether the
     * lookahead token can match the argument.
     * But it will not throw error if match fails.
     */
    inline bool check(TYPE);

    //This method will read the next token;
    inline void Lookahead();

    //Get the name of an identifier token.
    std::string get_name();

    //Get the value of a number token.
    int get_value();
    /*
     * Methods: Parsers()
     * --------------------------------------------------------------
     * Those parsers are defined according to the contex-free grammar
     * Those parser will call each other to achive the induction of
     * the grammar. The contex-free grammar will be given in the report
     * of the project.
     */
    // Check the definitions of global variables
    bool global_parser();
    //A program may have many functions.
    void program_parser();

    //Each function should have return type, (, arguments, ), {, Compound, }
    void function_declaration_parser(std::shared_ptr<Compound> &);

    //This function record the return type and name of the functions.
    std::shared_ptr<Compound> function_parser(); //<declaration_function> '{' 'Compound' '}'
    //A compound consists of many statements. It will keep getting token until } is found.
    void compound_parser(std::shared_ptr<Compound> &);

    // 6 types of statements (for, if, while, declaration, assignment, declaration+assignment)
    // are designed.
    void statement_parser(std::shared_ptr<Compound> &);

    //Not used for now.
    std::shared_ptr<Statement> bool_parser();

    // An expression is consists of +,-, term.
    std::shared_ptr<Statement> expression_parser();

    // A term consists of *,/, factor
    std::shared_ptr<Statement> term_parser();

    // A factor may be a number, an ID, a (.
    std::shared_ptr<Statement> factor_parser();

    //-----------------------------------------------------------------------------------------
    /*
     * Methods: AST_Traversalers
     * -----------------------------
     * Those methods will post-order traverse the tree
     * and print the result to check whether the mathematical
     * operations are conducted in a correct oreder.
     * ---------------------------------------------------
     */
    void COMPOUND_traversal(std::shared_ptr<Compound> &);

    void ASSIGNMENT_traversal(std::shared_ptr<Operator> &);

    void PRINTF_traversal(std::shared_ptr<Operator> &);

    void WHILE_traversal(std::shared_ptr<Operator> &);

    void algo_traversal(std::shared_ptr<Operator> &);

    void IF_traversal(std::shared_ptr<Operator> &);

};

struct MyException : public std::exception{
    std::string s;
    MyException(std::string ss) : s(ss){}
    ~MyException() throw (){}
    const char* what() const throw(){return s.c_str();}
};


#endif //SYNTACTIC_PARSER_H
