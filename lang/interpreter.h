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
        inline static std::vector<const char*>* defined = new std::vector<const char*>;
        static bool is_defined(const char* c);
        inline static int VERSION_MAJOR = 0;
        inline static int VERSION_MINOR = 1;

        inline static stack_frame* global_frame = new stack_frame();
        inline static std::vector<stack_frame*>* stack = nullptr;
        inline static std::stack<std::string>* errors = nullptr;

        inline static scanner* scan = nullptr;
        inline static bool has_init = false;
        inline static std::stack<bool>* if_block_statuses = nullptr;
        inline static std::stack<bool>* if_results = nullptr;

        static void init();

        // deprecated
        static char* const_char_convert(const char* input) {
            char* name = (char*)malloc(sizeof(char)*strlen(input));
            return strcpy(name, input);
        }

        static std::shared_ptr<token_group> evaluate_tokens(std::vector<std::shared_ptr<token>> tokens, int offset);
        static void input_loop();
        static int get_equal_index(const std::vector<std::shared_ptr<token>> &tokens);
        static std::vector<int> get_flags(const std::vector<std::shared_ptr<token>> &tokens);
        static void process_variable_declaration(const std::vector<std::shared_ptr<token>> &tokens);

        static bool set_literal(const std::vector<std::shared_ptr<token>> &tokens, data *d);

        static void process_variable_update(const std::vector<std::shared_ptr<token>> &tokens);

        static void process(const std::vector<std::shared_ptr<token>>& tokens);

        static void check_pop_stack(std::vector<std::shared_ptr<token>>& tokens);
        static void process_input(std::string* input);
        static void read_from_file(const char* path);
        static void error(const std::string& err);
        static void print_errs();

    };
}
#endif //INTERPRETER_H
