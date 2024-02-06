#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <iostream>
#include <stack>
#include <string.h>
#include <string>

#include "tokenizer/scanner.h"

class stack_frame;

namespace lang {
    class scanner;

    class interpreter {
    public:
        inline static std::stack<stack_frame*>* stack = nullptr;

        static char* const_char_convert(const char* input) {
            char* name = (char*)malloc(sizeof(char)*strlen(input));
            return strcpy(name, input);
        }
        static void input_loop();
        static void process_variable_declaration(const std::vector<token*> &tokens);
        static void process_variable_update(const std::vector<token*> &tokens);
        static void print(const std::vector<token*>& tokens);
        static void process(const std::vector<token*>& tokens);
        static void process_input(scanner& scan, std::string* input);
        static void read_from_file(const char* path);
        static void error(std::string err);

    };
}
#endif //INTERPRETER_H
