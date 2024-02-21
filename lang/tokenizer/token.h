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
// /i
#define EXPONENT 27
#define ID_GRAB 28
// &
#define MOD 29

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
#define PROC 111
#define NOTHING_TYPE (-2)
#define TYPE 113

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
#define PROC_KEYW 1013
#define BYVAL 1017
#define DISCARD 1018
#define PERSISTENT 1019
// I think persistent is a good word because something like global or heap wouldn't make sense
// Everything is heap, and global could work, but I don't like it.
#define END_IF 1020
#define END_PROC 1021
#define END_LOOP 1022
#define FINAL 1023
#define BREAK 1024
#define CONTINUE 1025
#define DO_WHILE 1026
#define UNTIL 1027
#define DO_UNTIl 1028

// TYPEWORDS
#define INT_KEYW 1120
#define FLOAT_KEYW 1121
#define DOUBLE_KEYW 1122
#define LONG_KEYW 1123
#define STRING_KEYW 1124
#define CHAR_KEYW 1125
#define BOOL_KEYW 1126
#define ULONG64_KEYW 1127
#define NOTHING_KEYW 1128
#define TYPE_KEYW 1129

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
#define ID 10011
#define ASSERT 10012

#define CAST 11001 // DO NOT MARK THIS AS A BUILT-IN FUNC, IT WILL BREAK THE CAST SYSTEM

class token {
    int name;
    const char* lexeme;
    std::any value;
    int line;

public:
    token(token* t) : name(t->name), lexeme(t->lexeme), value(t->value), line(t->line) {}
    token(int name, const char *lexeme, int line, std::any value = nullptr)
        : name(name),
          lexeme(lexeme),
          value(std::move(value)),
          line(line) {
    }

    [[nodiscard]] int get_name() const {
        return name;
    }
    [[nodiscard]] int is_identifier() const {
        return name == IDENTIFIER;
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
        return name >= PRINT && name < CAST;
    }

    bool is_arithmetic() const {
        return (name >= PLUS && name <= SLASH) || name == EXPONENT || name == SLASHI || name == MOD;
    }

    bool is_add_sub() const {
        return name == PLUS || name == MINUS;
    }

    bool is_mul_div() const {
        return name == STAR || name == SLASH || name == SLASHI || name == MOD;
    }

    bool is_typeword() const {
        return name >= INT_KEYW && name <= NOTHING_KEYW;
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

    bool is_control_flow() const {
        return name == IF || name == END_IF || name == ELSE || name == ELSE_IF;
    }
    bool is_loop() const {
        return name == FOR || is_while_variation();
    }
    bool is_while_variation() const {
        return name == WHILE || name == DO_WHILE
        || name == DO_UNTIl || name == UNTIL;
    }

    static const std::string type_tostr(int i) {
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
            case NOTHING_TYPE:
                return "nothing";
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
            std::string val = std::to_string(std::any_cast<float>(value));
            val.erase(val.find_last_not_of('0') + 1, std::string::npos);
            return val;
        }
        else if(name == DOUBLE) {
            std::string val = std::to_string(std::any_cast<double>(value));
            val.erase(val.find_last_not_of('0') + 1, std::string::npos);
            return val;
        }
        else if(name == LONG) {
            return std::to_string(std::any_cast<long>(value));
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
    int typeword_to_type() const {
        switch (name) {
            case INT_KEYW:
                return INT;
            case FLOAT_KEYW:
                return FLOAT;
            case DOUBLE_KEYW:
                return DOUBLE;
            case LONG_KEYW:
                return LONG;
            case STRING_KEYW:
                return STRING;
            case ULONG64_KEYW:
                return ULONG64;
            case NOTHING_KEYW:
                return NOTHING_TYPE;
            case BOOL_KEYW:
                return BOOL_KEYW;
            default:
                return 0;
        }
    }
};



#endif //TOKEN_H
