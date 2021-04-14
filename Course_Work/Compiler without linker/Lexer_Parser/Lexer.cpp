/*
 * Lexer.cpp
 * Ziqi Gao 118010077
 * ---------------------------------
 * This file is the implementation of Lexer
 * You can check implementation notes for
 * methods.
 * ----------------------------------------
 */

//Implementation of Lexer class:
#include <regex>
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <fstream>
#include <cctype>
#include <QDebug>
#include <sstream>
#include "Lexer.h"
#include "Global.h"

//Define and initialize the regular expressions
//-------------------------------------------------------------------------------------------------------
//Identifier: Started with one letter or underscore, then number, letter or underscore. Maximum length:31
const std::regex reg_identifier("[a-zA-Z_][a-z0-9A-Z_]{0,30}");
//Numbers in C can be a float, integer. It should also supports scientific notation.
const std::regex reg_Numbers("[0-9]+(.[0-9]+)?(E[+-]?[0-9]+)?", std::regex_constants::icase);
//Define operators:
const std::regex reg_OP("(\\+\\+|--|==|>=|<=|!=|<<|>>|!=|\\+=|-=|\\*=|\\/=|%=|\\+|-|\\*|%|=)");
//Define Reserved Words
const std::regex reg_Reserved("(if|else|when|braek|continue|for)");
//---------------------------------------------------------------------------------------------------------

//Initialization of Tokens data structure
std::vector<std::vector<std::shared_ptr<Token>>> allToken;
std::unordered_map<int, std::string> tag_table;
int Lexer::line = 1;
int Lexer::total_line = 1;

Lexer::Lexer() {
    using std::shared_ptr;
    //In the constructor of Lexer, we need to set reserved words at first.
    //Save the reserved words.
    shared_ptr<Word> rw1(new Word("if", TYPE::IF));
    shared_ptr<Word> rw2(new Word("else", TYPE::ELSE));
    shared_ptr<Word> rw3(new Word("while", TYPE::WHILE));
    shared_ptr<Word> rw4(new Word("do", TYPE::DO));
    shared_ptr<Word> rw5(new Word("break", TYPE::BREAK));
    shared_ptr<Word> rw6(new Word("for", TYPE::FOR));
    shared_ptr<Word> rw7(new Word("continue", TYPE::CONTINUE));
    shared_ptr<Word> rw8(new Type("int", TYPE::INT, 4));
    shared_ptr<Word> rw9(new Type("float", TYPE::FLOAT, 8));
    shared_ptr<Word> rw10(new Type("char", TYPE::CHAR, 1));
    shared_ptr<Word> rw11(new Type("void", TYPE::VOID, 0));
    shared_ptr<Word> rw12(new Word("printf", PRINTF));

    //Push those pointers about reserved words to hash_table.
    hash_table.insert({rw1->lexemes, std::move(rw1)});
    hash_table.insert({rw2->lexemes, std::move(rw2)});
    hash_table.insert({rw3->lexemes, std::move(rw3)});
    hash_table.insert({rw4->lexemes, std::move(rw4)});
    hash_table.insert({rw5->lexemes, std::move(rw5)});
    hash_table.insert({rw6->lexemes, std::move(rw6)});
    hash_table.insert({rw7->lexemes, std::move(rw7)});
    hash_table.insert({rw8->lexemes, std::move(rw8)});
    hash_table.insert({rw9->lexemes, std::move(rw9)});
    hash_table.insert({rw10->lexemes, std::move(rw10)});
    hash_table.insert({rw11->lexemes, std::move(rw11)});
    hash_table.insert({rw12->lexemes, std::move(rw12)});

    //Initialize tag_table
    tag_table.insert({256, "break"});
    tag_table.insert({257, "do"});
    tag_table.insert({258, "if"});
    tag_table.insert({259, "else"});
    tag_table.insert({260, "=="});
    tag_table.insert({261, ">="});
    tag_table.insert({262, "<="});
    tag_table.insert({263, "!="});
    tag_table.insert({264, ">="});
    tag_table.insert({265, "<"});
    tag_table.insert({266, "&&"});
    tag_table.insert({267, "||"});
    tag_table.insert({268, "for"});
    tag_table.insert({269, "while"});
    tag_table.insert({270, "Number"});
    tag_table.insert({271, "Identifier"});
    tag_table.insert({272, "continue"});
    tag_table.insert({273, ";"});
    tag_table.insert({274, "++"});
    tag_table.insert({275, "--"});
    tag_table.insert({276, "%"});
    tag_table.insert({277, "-"});
    tag_table.insert({278, "+"});
    tag_table.insert({279, "*"});
    tag_table.insert({280, "/"});
    tag_table.insert({281, "="});
    tag_table.insert({282, "int"});
    tag_table.insert({283, "float"});
    tag_table.insert({284, "char"});
    tag_table.insert({285, "("});
    tag_table.insert({286, ")"});
    tag_table.insert({287, "{"});
    tag_table.insert({288, "}"});
    tag_table.insert({289, "["});
    tag_table.insert({290, "]"});
    tag_table.insert({291, "+="});
    tag_table.insert({292, "-="});
    tag_table.insert({293, "*="});
    tag_table.insert({294, "-="});
    tag_table.insert({295, "EOF"});
    tag_table.insert({296, "void"});
    tag_table.insert({297, ","});
    tag_table.insert({298, "printf"});
    tag_table.insert({299, "PROGRAM"});
    tag_table.insert({300, "COMPOUND"});

}

/*
 * Method: Scanner
 * -----------------------------------------------
 * Implementation Notes: This method is the key method
 * in Lexer Class, which will divide the source code file
 * into different tokens that may be Word/Num/Type/Token.
 * And save those tokens' pointers into the 2D vector
 * based on the line number.
 * This process is done by check character one by one.
 * ----------------------------------------------------
 */
void Lexer::Scanner(std::vector<char> &buf) {
    //Namespace declaration:
    using std::string;
    using std::regex_match;
    auto end = buf.end();
    for (auto iter = buf.begin(); iter != buf.end();) {
        //Read character one by one:
        //At the beginning, skip the white space (ws, \t, \n)
        Whitespace_Lexer(iter, end);
        //Then, check whether it is an operator (e.g. + - * / >= !=)
        operator_Lexer(iter, end);
        //Check whether it is a constant number. (e,g, 15 1E500 2.55)
        Num_Lexer(iter, end);
        //Finally, check whether it is a variable's name or a reserved word (e.g. ABC_N, for, while)
        lexeme_Lexer(iter, end);
    }
//    Put an EOF to represent the end of the file.
    std::shared_ptr<Token> eof_ptr(new Token(END));
    allToken[line - 1].push_back(std::move(eof_ptr));
}

/*
 * Method: Precompile
 * -----------------------------------------------
 * Implementation Notes: This method remove the
 * notations in the source code file. It will also
 * record the number of instructions to allocate
 * memory to save the tokens.
 * -----------------------------------------------
 */
void Lexer::preCompile() {
    //Remove the comments from the source file:
    std::ifstream infile;
    std::ofstream outfile;
    infile.open(read_file.c_str());
    outfile.open(precompile_file.c_str());
    removeComments(infile, outfile);
    infile.close();
    outfile.close();
}

/*
 * Method: Precompile
 * -----------------------------------------------
 * Implementation Notes: This method read the precompiled file
 * and call Scanner to get all the tokens in the file.
 * -----------------------------------------------
 */
void Lexer::readFile() {
    using std::vector;
    std::ifstream PreCompileFile;
    //Read file by buffer and save the sentences into vecotr<char> buf.
    PreCompileFile = std::ifstream(precompile_file, std::ios::binary);
    vector<char> buf(static_cast<unsigned int>(PreCompileFile.seekg(0, std::ios::end).tellg()));
    PreCompileFile.seekg(0, std::ios::beg).read(&buf[0], static_cast<std::streamsize>(buf.size()));
    PreCompileFile.close();
    //Parse the code into different tokens.
    Scanner(buf);
}


void Lexer::Whitespace_Lexer(std::vector<char>::iterator &iter, std::vector<char>::iterator &end) {
    while (iter != end) {
        if (*iter == ' ' || *iter == '\t') {
            ++iter;
            continue; //Skip whitespace or tab
        }
            //Windows: \r\n, Linux: \n
        else if (*iter == '\r') ++iter;
        else if (*iter == '\n') {
            ++iter;
            ++line;
        } else break;
    }
}

/*
 * Implementation notes: Num_Lexer()
 * -----------------------------------
 * This function will keep read possible digits for a number.
 * Then it use regular expression to match the formulated string.
 *
 * To make this parser more functional, it is possible to check
 * whether the token is a float or an integer.
 *
 * Also, note that a negative number will not considered as one token.
 * Instead, each of negative number has a Minus operator token following
 * a Number token that can be recognized in the lexical analysis.
 * ------------------------------------------------------------
 */
void Lexer::Num_Lexer(std::vector<char>::iterator &iter, std::vector<char>::iterator &end) {
    if (iter == end) return;

    std::string temp = ""; //Use this variable to record the whole number.
    //If a token is a number, it must start with one digit
    if (isdigit(*iter)) {
        //Keep getting digits until we meet a space/tab/newline/semi column
        while (isdigit(*iter) || *iter == 'e' || *iter == 'E' || *iter == '.') {
            temp = temp + (*iter);
            ++iter;
        }
    } else return;
    //Use regular expression to check whether this token is valid or not.
    bool match = std::regex_match(temp, reg_Numbers);
    if (match) {
        std::stringstream str(temp);
        int value = 0;
        str >> value;
        std::shared_ptr<Token> ptr(new Num(value));
        allToken[line].push_back(std::move(ptr));
    } else std::cerr << "Received wrong Number declaration at line:" << line << std::endl;
}

void Lexer::lexeme_Lexer(std::vector<char>::iterator &iter, std::vector<char>::iterator &end) {
    if (iter == end) return;
    std::string temp = "";
    //An identifier/reserved word should start with a letter.
    if (isalpha(*iter)) {
        //Keep getting digits until we meet something other than digits, letters, and underscore
        while (isalnum(*iter) || *iter == '_') {
            temp = temp + (*iter);
            ++iter;
        }
    } else return;

    //Use Regular Expression to check whether this token is valid or not.
    bool match = std::regex_match(temp, reg_identifier);
    if (match) {
        //If you cannot find the string in the reserved word map,
        //This string should be an normal identifier.
        if (hash_table.find(temp) == hash_table.end()) {
            std::shared_ptr<Token> ptr(new Word(temp, ID));
            allToken[line].push_back(std::move(ptr));
        }
            //If so, this string is a reserved word.
        else {
            auto got = hash_table.find(temp);
            TYPE word_type = got->second->tag;
            std::shared_ptr<Token> ptr(new Word(temp, word_type));
            allToken[line].push_back(std::move(ptr));
        }
    } else{
        qDebug() << "Recived wrong identifier at line:" << line;
        std::cerr << "Received wrong identifier at line: " << line << std::endl;
        }
}

/*
 * Implementation notes: operator_Lexer()
 * ---------------------------------------
 * This function parse the operators
 * that will be used later.
 * It works like a transition map to match
 * the largest string and decide which operator it is then.
 * -------------------------------------------------------
 */

void Lexer::operator_Lexer(std::vector<char>::iterator &iter, std::vector<char>::iterator &end) {
    if (iter == end) return;
    switch (*iter) {
        case ';': {
            std::shared_ptr<Token> ptr(new Token(SEMICOLON));
            allToken[line].push_back(std::move(ptr));
            ++iter;
            break;
        }
        case '+': {
            ++iter;
            //++
            if (*iter == '+') {
                std::shared_ptr<Token> ptr(new Token(INC));
                allToken[line].push_back(std::move(ptr));
                ++iter;
            }
                //+=
            else if (*iter == '=') {
                std::shared_ptr<Token> ptr(new Token(PE));
                allToken[line].push_back(std::move(ptr));
                ++iter;
            }
                //+
            else {
                std::shared_ptr<Token> ptr(new Token(PLUS));
                allToken[line].push_back(std::move(ptr));
            }
            break;
        }

        case '-': {
            ++iter;
            //--
            if (*iter == '-') {
                std::shared_ptr<Token> ptr(new Token(DEC));
                allToken[line].push_back(std::move(ptr));
                ++iter;
            }
                //-=
            else if (*iter == '=') {
                std::shared_ptr<Token> ptr(new Token(SE));
                allToken[line].push_back(std::move(ptr));
                ++iter;
            }
                //-
            else {
                std::shared_ptr<Token> ptr(new Token(MINUS));
                allToken[line].push_back(std::move(ptr));
            }
            break;
        }

        case '*': {
            ++iter;
            //*=
            if (*iter == '=') {
                std::shared_ptr<Token> ptr(new Token(ME));
                allToken[line].push_back(std::move(ptr));
                ++iter;
            }
                //*
            else {
                std::shared_ptr<Token> ptr(new Token(MUL));
                allToken[line].push_back(std::move(ptr));
            }
            break;
        }

        case '/': {
            ++iter;
            // /=
            if (*iter == '=') {
                std::shared_ptr<Token> ptr(new Token(DE));
                allToken[line].push_back(std::move(ptr));
                ++iter;
            }
                // / (divide)
            else {
                std::shared_ptr<Token> ptr(new Token(DIV));
                allToken[line].push_back(std::move(ptr));
            }
            break;
        }

        case '%': {
            std::shared_ptr<Token> ptr(new Token(REMAIN));
            allToken[line].push_back(std::move(ptr));
            break;
        }
        case '=': {
            ++iter;
            //==
            if (*iter == '=') {
                std::shared_ptr<Token> ptr(new Token(EQ));
                allToken[line].push_back(std::move(ptr));
                ++iter;
            }
                // = (Assignment)
            else {
                std::shared_ptr<Token> ptr(new Token(ASSIGNMENT));
                allToken[line].push_back(std::move(ptr));
            }
            break;
        }
        case '>': {
            ++iter;
            // >=
            if (*iter == '=') {
                std::shared_ptr<Token> ptr(new Token(GE));
                allToken[line].push_back(std::move(ptr));
                ++iter;
            }
                // >
            else {
                std::shared_ptr<Token> ptr(new Token(GT));
                allToken[line].push_back(std::move(ptr));
            }
            break;
        }
        case '<': {
            ++iter;
            // <=
            if (*iter == '=') {
                std::shared_ptr<Token> ptr(new Token(LE));
                allToken[line].push_back(std::move(ptr));
                ++iter;
            }
                // <
            else {
                std::shared_ptr<Token> ptr(new Token(LT));
                allToken[line].push_back(std::move(ptr));
            }
            break;
        }
            //The following part still need to be improved for string cases.
        case '!': {
            ++iter;
            // !=
            if (*iter == '=') {
                std::shared_ptr<Token> ptr(new Token(NEQ));
                allToken[line].push_back(std::move(ptr));
                ++iter;
            }
            break;
        }
        case '&': {
            ++iter;
            // &&
            if (*iter == '&') {
                std::shared_ptr<Token> ptr(new Token(AND));
                allToken[line].push_back(std::move(ptr));
            }
            break;
        }
        case '|': {
            ++iter;
            // ||
            if (*iter == '&') {
                std::shared_ptr<Token> ptr(new Token(OR));
                allToken[line].push_back(std::move(ptr));
            }
            break;
        }
        case '{': {
            ++iter;
            std::shared_ptr<Token> ptr(new Token(LCB));
            allToken[line].push_back(std::move(ptr));
            break;

        }
        case '}': {
            ++iter;
            std::shared_ptr<Token> ptr(new Token(RCB));
            allToken[line].push_back(std::move(ptr));
            break;

        }
        case '(': {
            ++iter;
            std::shared_ptr<Token> ptr(new Token(LB));
            allToken[line].push_back(std::move(ptr));
            break;

        }
        case ')': {
            ++iter;
            std::shared_ptr<Token> ptr(new Token(RB));
            allToken[line].push_back(std::move(ptr));
            break;
        }
        case '[': {
            ++iter;
            std::shared_ptr<Token> ptr(new Token(LSB));
            allToken[line].push_back(std::move(ptr));
            break;
        }
        case ']': {
            ++iter;
            std::shared_ptr<Token> ptr(new Token(RSB));
            allToken[line].push_back(std::move(ptr));
            break;
        }
    }
}

/*
 * Implementation notes: removeComments()
 * --------------------------------------
 * This function will read the code
 * in the input file line by line,
 * and skip the content of comments.
 * Then it write the code without comment into the output file. (Come from Assignment 1 Q3)
 * ---------------------------------------
 */
void Lexer::removeComments(std::istream &is, std::ostream &os) {
    using std::string;
    using std::endl;
    string line;
    string temp;
    size_t location;
    size_t start = 0;
    while (!is.eof()) {
        ++total_line;
        getline(is, line);
        //Deal with the problem of whitespace first.
        start = line.find_first_not_of(" ");
        //Situation 1: If this line contents with "*/"
        if (line.find("*/") != string::npos) {
            //Plus 2 to eliminate "*/".
            location = line.find("*/") + 2;
            //Get the content after */
            line = line.substr(location);
            os << line << endl;
            continue;
        }
        //Situation 2: If this line starts with "//" or "*" (not "*/") just skip it.
        if (line.find("*", start) == start || line.find("//", start) == start) {
            os << "" << endl;
            continue;
        }

        //Situation 3: If this line starts with "/*"
        if (line.find("/*") != string::npos) {
            //If /* and */ do not occur in one line, skip this line directly.
            if (line.find("*/") == string::npos) {
                os << "" << endl;
                continue;
            }
        }

        //Situation 4: If "//"Not in the beginning of this line.
        if (line.find("//") != string::npos) {
            location = line.find("//");
            line = line.substr(0, location);
            os << line << endl;
            continue;
        }
        //Situation 5: No comment in this line.
        os << line << endl;
    }
}

void Lexer::printToken() {
    using std::endl;
    int count = 1;
    for(auto line = allToken.begin()+1; line != allToken.end(); ++line){
        std::cout << "Line " << count << ": "<< endl;
        result_file << "Line " << count << ": "<< endl;
        for(auto iter_token = (*line).begin(); iter_token != (*line).end(); ++iter_token){
           std::cout << tag_table[(*iter_token)->tag] << ", ";
            result_file << tag_table[(*iter_token)->tag] << ", ";
        }
        std::cout << endl;
        result_file << endl;
        ++count;
    }
}
