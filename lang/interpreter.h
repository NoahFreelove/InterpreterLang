#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <iostream>
#include <stack>
#include <cstring>

#include "tokenizer/scanner.h"
#include "memory/memory.h"
#include "tokenizer/token_group.h"

namespace lang {
    class scanner;

    class interpreter {
    public:
        inline static std::vector<char*>* defined = new std::vector<char*>;

        inline static stack_frame* global_frame = new stack_frame();
        inline static std::stack<stack_frame*>* stack = nullptr;
        inline static scanner* scan = nullptr;
        inline static bool has_init = false;

        static void init();
        static char* const_char_convert(const char* input) {
            char* name = (char*)malloc(sizeof(char)*strlen(input));
            return strcpy(name, input);
        }
        static token_group* evaluate_tokens(std::vector<token*> tokens, int offset);
        static void input_loop();
        static void process_variable_declaration(const std::vector<token*> &tokens);

        static bool set_literal(const std::vector<token *> &tokens, data *d);

        static void process_variable_update(const std::vector<token*> &tokens);

        static void process(const std::vector<token*>& tokens);
        static void process_input(std::string* input);
        static void read_from_file(const char* path);
        static void error(std::string err);

    };
}
#endif //INTERPRETER_H
