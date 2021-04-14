#include <iostream>
#include <unordered_map>
#include <memory>
#include "env.h"
#include "Lexer.h"
Env::Env() {
    prev = nullptr;
}

Env::Env( Env & previous){
    prev = &previous;
}

bool Env::get_id(std::shared_ptr<Token> Token) {
    auto temp = std::static_pointer_cast<Word>(Token);
    std::string name = temp->lexemes;
    for(Env* e = this; e != nullptr; e = e->prev){
        //If token is found:
        if(e->table.find(name) != e->table.end()){
            return true;
        }
    }
    return false;
}
