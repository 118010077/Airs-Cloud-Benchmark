#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <regex>
#include "Global.h"
#include "Lexer_Parser/Lexer.h"
#include "Lexer_Parser/Parser.h"
#include "Intermediate_Code_Generator/defines.h"
#include "Intermediate_Code_Generator/c_to_mips.h"
#include "Assembler_Simulator/Assembler_main.h"


std::string read_file;
const std::string precompile_file = "pre.txt";
const std::string assemble_file = "asm.txt";
const std::string machine_file = "machine.o";
std::ofstream result_file;

void compiler(std::string sourceCode) {
    using std::cout;
    using std::endl;
    read_file = sourceCode;
    result_file.open("result.txt");
    std::ofstream asm_file;
    asm_file.open(assemble_file.c_str());
    std::ifstream sourceFile;
    std::ofstream precompileFile;
    //Part 0: Pre-compile to remove notations
    Lexer lex;
    Lexer::preCompile();
    allToken.resize(Lexer::total_line); //Set the maximum lines of instructions.
    //Part 1: ReadFile and get tokens (lexical analysis)
    lex.readFile();
    cout << "Lexical Analysis Finished" << endl;
    result_file << "Lexical Analysis Finished" << endl;
    cout << "Tokens in each line:" << endl;
    result_file << "Tokens in each line:" << endl;
    Lexer::printToken();
    cout << endl;
    result_file << endl;
    //Part 2: Parser (Syntactic Analysis) and generate AST.
    ::Parser parser;
    parser.syntactic_analysis();
    cout <<"Syntactical Analysis Finished" << endl << endl;
    result_file << "Syntactical Analysis Finished" << endl;
    result_file << endl;
    //Print by AST:
    parser.AST_traversal();
    //Part 3: Translator
    cout << "Before function translate" << endl;
    result_file << "Before function translate" << endl;
    std::string s = "L_0";
    std::string mips = translate(file1);
    cout << mips << endl << endl;
    result_file << mips << endl;
    asm_file << mips << endl;
    //Part 4: Assembler and Simulator
    cout << "Content in .o file" << endl;
    result_file << "Content in .o file:" << endl;
    Assemble();
    result_file << "Finish" << endl;
    cout << "Finish" << endl;
    result_file.close();
    return;

}

