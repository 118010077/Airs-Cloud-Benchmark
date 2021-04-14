#ifndef ENV_H
#define ENV_H
#include "Token.h"
#include "Program.h"
#include <unordered_map>

class Env {
public:
    Env();
    Env(Env &previous_table);
    std::unordered_map<std::string, int> table;
    Env  *prev;
    bool get_id(std::shared_ptr<Token> Token);
};


#endif // ENV_H
