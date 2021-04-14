/*
 * Filename: Token.h
 * ----------------------------------------------------
 * This file define class Token and its derived classes:
 * Word, Type, Num
 * -----------------------------------------------------
 */
#ifndef SYNTACTIC_TOKEN_H
#define SYNTACTIC_TOKEN_H

#include <iostream>

//Since ASCII has defined characters in 0-255.
//Start from 256:
enum TYPE {
    BREAK = 256, DO = 257, IF = 258, ELSE = 259,
    EQ = 260, GE = 261, LE = 262, NEQ = 263, //==, >=, <=, !=
    GT = 264, LT = 265, //>=, <=
    AND = 266, OR = 267, //&&, ||
    FOR = 268, WHILE = 269,
    NUM = 270, ID = 271, CONTINUE = 272, SEMICOLON = 273,
    INC = 274, DEC = 275, REMAIN = 276,            //++, --, %
    MINUS = 277, PLUS = 278, MUL = 279, DIV = 280, // - + * /
    ASSIGNMENT = 281, // ==
    INT = 282, FLOAT = 283, CHAR = 284, //Three Data Types
    LB = 285, RB = 286, LCB = 287, RCB = 288, LSB = 289, RSB = 290, //(, ), {, }, [, ]
    PE = 291, SE = 292, ME = 293, DE = 294, END = 295,   //END: End of the file.
    VOID = 296, COMMA = 297,PRINTF = 298,
    //Following are some non terminals
    PROGRAM, COMPOUND
}; //+=, -=, *=, /=


/* Class Name: Token
 * Usage: shared_ptr<Token> ptr(new Token(TYPE))
 * ---------------------------------------------------------
 * This is the definition of Token. All the tokens will be saved
 * in the heap memory and linked by the smart pointers.
 * ----------------------------------------------------------
 */
class Token {
public:
    Token(TYPE token_type) { tag = token_type; }
    TYPE tag;
};

/* Class Name: Word
 * ---------------------------------------------------------
 * Word is a subclass of Token, which represents identifiers,
 * operators, and reserved words in C.
 * ----------------------------------------------------------
 */
class Word : public Token {
public:
    Word(std::string lexemes_r, TYPE token_type) : Token(token_type) {
        lexemes = lexemes_r;
    }

    std::string lexemes;
};

/* Class Name: Type
 * ---------------------------------------------------------
 * Type is a subclass of Word, which represents data type like float,
 * int, double.... It has another element: width to set the memory space
 * ----------------------------------------------------------
 */
class Type : public Word {
public:
    Type(std::string lexemes_r, TYPE token_type, int w = 4) : Word(lexemes_r, token_type), width(w) {}

    int width;
};

/* Class Name: Num
 * ---------------------------------------------------------
 * Num is also a subclass of Token, which represents constant
 * numbers that can be used in assignment or calculation.
 * ----------------------------------------------------------
 */
class Num : public Token {
public:
    Num(int value_r, TYPE token_type = NUM) : Token(token_type), value(value_r) {}
    int value;
};

#endif //SYNTACTIC_TOKEN_H
