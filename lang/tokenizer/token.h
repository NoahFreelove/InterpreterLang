#ifndef TOKEN_H
#define TOKEN_H
#include <any>
#include <utility>
#include <ostream>

#include "id_to_name.h"

// OPS
#define LEFT_PAREN 1
#define RIGHT_PAREN 2
#define LEFT_BRACE 3
#define RIGHT_BRACE 4
#define LEFT_BRACKET 5
#define RIGHT_BRACKET 6
#define COMMA 7
#define DOT 8
#define PLUS 9
#define MINUS 10
#define STAR 11
#define SLASH 12
#define SEMICOLON 13
#define BANG 14
// !
#define EQUAL 15
#define DEQUAL 16
// ==
#define TEQUAL 17
// ===
#define GT 18
// >
#define LT 19
// <
#define GTE 20
// >=
#define LTE 21
// <=
#define ID_GRAB 22
// &
#define BANG_EQUAL 23
#define SLASHI 24
#define AND 25
#define OR 26
#define XOR 27
#define EXPONENT 28
// /i

// LITERALS
#define IDENTIFIER 101
#define STRING 102
#define FLOAT 104
#define DOUBLE 105
#define LONG 106
#define ULONG64 107
#define INT 108

// KEYWORDS
#define IF 1001
#define ELSE 1002
#define ELSE_IF 1003
#define FOR 1004
#define WHILE 1005
#define STRUCT 1006
#define CLASS 1007
#define PRIVATE 1008
#define PUBLIC 1009
#define TRUE 1010
#define FALSE 1011
#define RETURN 1012
#define FUNCTION 1013
#define NULL_LANG 1014
#define VAR 1015
#define AS 1016
#define BYVAL 1017
#define DISCARD 1018
#define PERSISTENT 1019

// builtin funcs
#define PRINT 10001

class token {
    int name;
    const char* lexeme;
    std::any value;
    int line;

public:
    token(int name, const char *lexeme, int line, std::any value = nullptr)
        : name(name),
          lexeme(lexeme),
          value(std::move(value)),
          line(line) {
    }

    [[nodiscard]] int get_name() const {
        return name;
    }

    [[nodiscard]] const char * get_lexeme() const {
        return lexeme;
    }

    [[nodiscard]] std::any get_value() const {
        return value;
    }
    [[nodiscard]] int get_line() const {
        return line;
    }

    // generate iostream operator
    friend std::ostream &operator<<(std::ostream &os, const token &token) {
        os << "name: " << id_to_name(token.name) << " lexeme: " << token.lexeme << " value?: " << (token.value.type() != typeid(nullptr)) << " line: " << token.line;
        return os;
    }
};



#endif //TOKEN_H
