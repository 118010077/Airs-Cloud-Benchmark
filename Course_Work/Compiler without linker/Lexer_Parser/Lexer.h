/*
 * File Name: Lexer.h
 * -------------------
 * This file define the class of Lexer that
 * will precompile the file to remove the notations.
 * Then, it will read the precompiled file to
 * save tokens' shared pointers in a 2-D vector.
 * For example, tokens in the ith line is saved in vector[i].
 */
//--------------------------------
#ifndef SYNTACTIC_LEXER_H
#define SYNTACTIC_LEXER_H

#include <memory>
#include <vector>
#include <unordered_map> //Hash set
#include "Token.h"
//--------------------------------


class Lexer {
public:
/*
 * Constructor: Lexer
 * Usage: Lexer() lex;
 * -------------------
 * This will create an lexer object that
 * has a hash_table to save all the reserved words.
 * This class also define methods to precompile the file and
 * parse the code into a 2D vector.
 */
    Lexer();

/*
 * Destructor: ~Lexer
 * Since we use smart pointers to manage Tokens,
 * there is no need to do extra operations to free the memory.
 */
    ~Lexer() {};

    void Scanner(std::vector<char> &);

    void readFile();

    static void preCompile();

    static void printToken();

    static int line; //This element can record tokens' locations.

    static int total_line; //This element record the total number of instructions.

private:
    //Set an hash_map to record the reserved words.
    std::unordered_map<std::string, std::shared_ptr<Word>> hash_table;

    static void removeComments(std::istream &, std::ostream &);

    //Parsers
    void Whitespace_Lexer(std::vector<char>::iterator &, std::vector<char>::iterator &);

    void Num_Lexer(std::vector<char>::iterator &, std::vector<char>::iterator &);

    void lexeme_Lexer(std::vector<char>::iterator &, std::vector<char>::iterator &);

    void operator_Lexer(std::vector<char>::iterator &, std::vector<char>::iterator &);
};

#endif //SYNTACTIC_LEXER_H
