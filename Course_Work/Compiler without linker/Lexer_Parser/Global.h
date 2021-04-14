/*
 * File name： Global.h
 * ---------------------
 * This file have all the declarations of the global variables
 * that will be used in the project.
 */
//-------------------------
#ifndef SYNTACTIC_GLOBAL_H
#define SYNTACTIC_GLOBAL_H

#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <memory>
#include "Token.h"
#include "Program.h"
//---------------------


extern std::string read_file; //Source File Name
extern const std::string precompile_file; //Precompile File Name
extern const std::string assemble_file; //Assemble File Name
extern const std::string machine_file; //Machine Code File Name
extern std::vector<std::vector<std::shared_ptr<Token>>> allToken; //This 2-d vector saved all tokens for each line.
//This pointer is the starting point of the AST node. The interpreter of MIPS will start from here.
extern std::shared_ptr<Program> file1;
extern std::unordered_map<int, std::string> tag_table;
extern std::ofstream result_file;
#endif //SYNTACTIC_GLOBAL_H
