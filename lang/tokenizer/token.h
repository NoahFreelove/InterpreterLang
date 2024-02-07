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
#define TRUE 109
#define FALSE 110

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
#define RETURN 1012
#define FUNCTION 1013
#define NULL_LANG 1014
#define VAR 1015
#define AS 1016
#define BYVAL 1017
#define DISCARD 1018
#define PERSISTENT 1019

// TYPEWORDS
#define INT_KEYW 1120
#define FLOAT_KEYW 1121
#define DOUBLE_KEYW 1122
#define LONG_KEYW 1123
#define STRING_KEYW 1124
#define CHAR_KEYW 1125
#define BOOL_KEYW 1126
#define ULONG64_KEYW 1127

// builtin funcs
#define PRINT 10001
#define DUMP 10002
#define INPUT 10003
#define TYPEOF 10004
#define SIZEOF 10005
#define DELETE 10006



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

    bool is_op() const {
        return name >= LEFT_PAREN && name <= EXPONENT;
    }

    bool is_literal() const {
        return name >= IDENTIFIER && name <= FALSE;
    }

    bool is_literal_non_id() const {
        return name > IDENTIFIER && name <= FALSE;
    }

    bool is_keyword() const {
        return name >= IF && name <= PERSISTENT;
    }

    bool is_builtin() const {
        return name >= PRINT;
    }

    bool is_arithmetic() const {
        return name >= PLUS && name <= EXPONENT;
    }

    bool is_typeword() const {
        return name >= INT_KEYW && name <= ULONG64_KEYW;
    }

    bool is_numeric() const {
        return name >= INT && name <= ULONG64;
    }

    bool is_truthy() const {
        return name == TRUE || name == FALSE;
    }

    friend std::ostream &operator<<(std::ostream &os, const token &token) {
        os << "name: " << id_to_name(token.name) << " lexeme: " << token.lexeme << " value?: " << (token.value.type() != typeid(nullptr)) << " line: " << token.line;
        return os;
    }
};



#endif //TOKEN_H
