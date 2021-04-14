#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <regex>
#include "Global.h"
#include "Lexer.h"
#include "Parser.h"

std::string read_file;
const std::string precompile_file = "pre.txt";
const std::string assemble_file = "asm.txt";
const std::string machine_file = "machine.txt";
std::ofstream result_file;

void compiler(std::string sourceCode) {
    read_file = sourceCode;
    using std::endl;
    result_file.open("result.txt");
    std::ifstream sourceFile;
    std::ofstream precompileFile;
    //Part 0: Pre-compile to remove notations
    Lexer lex;
    Lexer::preCompile();
    allToken.resize(Lexer::total_line); //Set the maximum lines of instructions.
    //Part 1: ReadFile and get tokens (lexical analysis)
    lex.readFile();
    Lexer::printToken();
    result_file << "Lexical Analysis Finished" << endl;
    //Part 2: Parser (Syntactic Analysis) and generate AST.
    Parser parser;
    parser.syntactic_analysis();
    result_file << "Syntactical Analysis Finished" << endl;
    parser.AST_traversal();
}


