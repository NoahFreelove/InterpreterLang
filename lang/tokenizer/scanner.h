#ifndef SCANNER_H
#define SCANNER_H
#include <vector>

#include "token.h"
#include "../interpreter.h"

namespace lang {

    class scanner {
        int line = 0;
        int current = 0;
        int end = 0;
        bool in_multi_line_expression = false;
        bool in_multi_str = false;
        bool in_multi_com = false;
        std::vector<token*> multi_line_expression = {};
        std::string* data;
        bool err = false;

        bool past_end() {
            return past_end(0);
        }

        bool past_end(int i) {
            return (current+i )> end;
        }

        bool at_end() {
            return current == end;
        }

        char last_char() {
            return data->c_str()[data->length()-1];
        }
        //TODO: If this breaks its because I replaces last_char() with \0
        char peek(int amount = 0) {
            if(past_end(amount))
                return '\0';
            return data->c_str()[current+amount];
        }

        std::string peek_str(const int amount) {
            if(past_end(amount))
                return "";
            std::string str;
            for (int i = 0; i < amount; ++i) {
                str += data->c_str()[current+i];
            }
            return str;
        }

        char next() {
            if(past_end()) {
                return last_char();
            }
            return data->c_str()[current++];
        }

        token* get_digit(char first_num);

        token * get_identifier(char c);

        void skip(int amount = 1);

        token* trigger_error(char c);

        token * get_string();

        token* parse_token();

    public:
        scanner();
        std::vector<token*> scan_line(std::string* data);
        bool in_multi_line() const;
        bool in_multi_string() const;
        bool in_multi_comment() const;
    };
}
#endif //SCANNER_H
