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
#define BANG_EQUAL 22
#define AND 23
#define OR 24
#define XOR 25
#define SLASHI 26
#define EXPONENT 27
#define ID_GRAB 28
// &
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
#define PROC 1013
#define VAR 1015
#define AS 1016
#define BYVAL 1017
#define DISCARD 1018
#define PERSISTENT 1019
#define END_IF 1020
#define END_PROC 1021
// I think persistent is a good word because something like global or heap wouldn't make sense
// Everything is heap, and global could work, but I don't like it.

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
#define DEFINE 10007
#define UNDEFINE 10008
#define ISDEFINED 10009
#define IMPORT 10010

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

    void set_value(std::any val) {
        value = std::move(val);
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
        return (name >= PLUS && name <= SLASH) || name == EXPONENT;
    }

    bool is_add_sub() const {
        return name == PLUS || name == MINUS;
    }

    bool is_mul_div() const {
        return name == STAR || name == SLASH;
    }

    bool is_typeword() const {
        return name >= INT_KEYW && name <= ULONG64_KEYW;
    }

    bool is_numeric() const {
        return name <= 108 && name >= 104;
    }

    bool is_truthy() const {
        return name == TRUE || name == FALSE;
    }

    bool is_logical() const {
        return name >= DEQUAL && name <= XOR;
    }

    bool is_DFI() const {
        return name == DOUBLE || name == FLOAT || name == INT;
    }

    bool is_FI() const {
        return name == FLOAT || name == INT;
    }

    bool is_LI() const {
        return name == LONG || name == INT;
    }

    bool is_comparison() const {
        return name == EQUAL || name == BANG_EQUAL || name == GT || name == LT || name == GTE || name == LTE;
    }

    bool is_connective() const {
        return name == AND || name == OR;
    }

    bool is_bool() const {
        return name == TRUE || name == FALSE;
    }

    bool is_first_order_logic() const {
        return name == OR || name == XOR || name == AND;
    }

    static const char* type_to_char(int i) {
        switch (i) {
            case DOUBLE: {
                return "double";
            }
            case FLOAT:
                return "float";
            case INT:
                return "int";
            case LONG:
                return "long";
            case STRING:
                return "string";
            case TRUE:
                return "bool";
            case FALSE:
                return "false";
            default:
                return "";
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const token &token) {
        os << "name: " << id_to_name(token.name) << " lexeme: " << token.lexeme << " value?: " << (token.value.type() != typeid(nullptr)) << " line: " << token.line;
        return os;
    }

    std::string to_string() {
        if(name == INT) {
            return std::to_string(std::any_cast<int>(value));
        }
        else if(name == FLOAT) {
            return std::to_string(std::any_cast<float>(value));
        }
        else if(name == DOUBLE) {
            return std::to_string(std::any_cast<double>(value));
        }
        else if(name == STRING) {
            return std::any_cast<std::string>(value);
        }
        else if(name == TRUE) {
            return "true";
        }
        else if(name == FALSE) {
            return "false";
        }
        else {
            return lexeme;
        }
    }
};



#endif //TOKEN_H
