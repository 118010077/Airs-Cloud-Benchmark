#ifndef C_TO_MIPS_H
#define C_TO_MIPS_H
#include "defines.h"
#include "Lexer_Parser/Program.h"
#include "Lexer_Parser/Token.h"
#include <iostream>
#include <string>
#include <map>


using std::shared_ptr;
std::string translate(std::shared_ptr<Program> root);
std::map<int, int> init_reverse_comparison_sign_map();
std::string translate_compound(std::shared_ptr<Compound> node);


#endif

