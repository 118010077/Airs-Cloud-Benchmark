
#ifndef DEFINES_H
#define DEFINES_H
#include <map>
#include <string>
#include <vector>
#include "Lexer_Parser/Program.h"
#include "Lexer_Parser/Token.h"

extern std::map<std::string, std::string> label_blocks;

std::string translate_ADD(std::shared_ptr<Statement> Node, std::map<std::string, std::string> &identifiers);
std::string translate_SUB(std::shared_ptr<Statement> Node, std::map<std::string, std::string> &identifiers);
std::string translate_MULT(std::shared_ptr<Statement> Node, std::map<std::string, std::string> &identifiers);
std::string translate_DIV(std::shared_ptr<Statement> Node, std::map<std::string, std::string> &identifiers);
std::string translate_IF(std::shared_ptr<Statement> Node, std::map<std::string, std::string> &identifiers, int count_label);
std::string translate_WHILE(std::shared_ptr<Statement> node, std::map<std::string, std::string> &identifiers, int count_label);
std::string translate_CONDITION(std::shared_ptr<Statement> node, std::map<std::string, std::string> &identifiers, int count_label);
std::map<int, int> init_reverse_comparison_sign_map();
std::string translate(std::shared_ptr<Program> root);
std::string translate_compound(std::shared_ptr<Compound> node);
int parse_source_file();


#endif
