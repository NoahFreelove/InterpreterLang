#include "id_to_name.h"
#include "token.h"

std::string id_to_name(int id) {
    switch (id) {
        // go through all the tokens and return the name of the token
        case LEFT_PAREN:
            return "LEFT_PAREN";
        case RIGHT_PAREN:
            return "RIGHT_PAREN";
        case LEFT_BRACE:
            return "LEFT_BRACE";
        case RIGHT_BRACE:
            return "RIGHT_BRACE";
        case LEFT_BRACKET:
            return "LEFT_BRACKET";
        case RIGHT_BRACKET:
            return "RIGHT_BRACKET";
        case COMMA:
            return "COMMA";
        case DOT:
            return "DOT";
        case MINUS:
            return "MINUS";
        case PLUS:
            return "PLUS";
        case STAR:
            return "STAR";
        case SLASH:
            return "SLASH";
        case SEMICOLON:
            return "SEMICOLON";
        case BANG:
            return "BANG";
        case EQUAL:
            return "EQUAL";
        case DEQUAL:
            return "DEQUAL";
        case TEQUAL:
            return "TEQUAL";
        case GT:
            return "GT";
        case LT:
            return "LT";
        case GTE:
            return "GTE";
        case LTE:
            return "LTE";
        case ID_GRAB:
            return "ID_GRAB";
        case BANG_EQUAL:
            return "BANG_EQUAL";
        case SLASHI:
            return "SLASHI";
        case MOD:
            return "MOD";
        case IDENTIFIER:
            return "IDENTIFIER";
        case STRING:
            return "STRING";
        case FLOAT:
            return "FLOAT";
        case DOUBLE:
            return "DOUBLE";
        case LONG:
            return "LONG";
        case ULONG64:
            return "ULONG64";
        case INT:
            return "INT";
        case TYPE:
            return "TYPE";
        case NOTHING_TYPE:
            return "NOTHING";
        case IF:
            return "IF";
        case END_IF:
            return "END_IF";
        case ELSE:
            return "ELSE";
        case ELSE_IF:
            return "ELSE_IF";
        case FOR:
            return "FOR";
        case WHILE:
            return "WHILE";
        case STRUCT:
            return "STRUCT";
        case CLASS:
            return "CLASS";
        case PRIVATE:
            return "PRIVATE";
        case PUBLIC:
            return "PUBLIC";
        case TRUE:
            return "TRUE";
        case FALSE:
            return "FALSE";
        case RETURN:
            return "RETURN";
        case BREAK:
            return "BREAK";
        case CONTINUE:
            return "CONTINUE";
        case PROC:
            return "PROC";
        case PROC_KEYW:
            return "PROC";
        case END_PROC:
            return "END_PROC";
        case END_LOOP:
            return "END_LOOP";
        case FINAL:
            return "FINAL";
        case BYVAL:
            return "BYVAL";
        case DISCARD:
            return "DISCARD";
        case PERSISTENT:
            return "PERSISTENT";
        case CAST:
            return "CAST";
        case AND:
            return "AND";
        case OR:
            return "OR";
        case XOR:
            return "XOR";
        case PRINT:
            return "PRINT";
        case DUMP:
            return "DUMP";
        case INPUT:
            return "INPUT";
        case RAWINPUT:
            return "RAWINPUT";
        case EXIT:
            return "EXIT";
        case TYPEOF:
            return "TYPEOF";
        case SIZEOF:
            return "SIZEOF";
        case DELETE:
            return "DELETE";
        case DEFINE:
            return "DEFINE";
        case UNDEFINE:
            return "UNDEFINE";
        case ISDEFINED:
            return "ISDEFINED";
        case IMPORT:
            return "IMPORT";
        case ID:
            return "ID";
        case EXPONENT:
            return "EXPONENT";
        case INT_KEYW:
            return "INT_KEYW";
        case FLOAT_KEYW:
            return "FLOAT_KEYW";
        case DOUBLE_KEYW:
            return "DOUBLE_KEYW";
        case LONG_KEYW:
            return "LONG_KEYW";
        case STRING_KEYW:
            return "STRING_KEYW";
        case CHAR_KEYW:
            return "CHAR_KEYW";
        case BOOL_KEYW:
            return "BOOL_KEYW";
        case ULONG64_KEYW:
            return "ULONG64_KEYW";
        case NOTHING_KEYW:
            return "NOTHING_KEYW";
        case TYPE_KEYW:
            return "TYPE_KEYW";
        default:
            return "UNKNOWN TOKEN";
    }
}