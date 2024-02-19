#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <chrono>
#include <iostream>
#include <stack>
#include <cstring>
#include <queue>

#include "tokenizer/scanner.h"
#include "memory/memory.h"
#include "tokenizer/token_group.h"
#include "memory/proc_manager.h"
namespace lang {
    class scanner;

    class interpreter {
    public:
        inline static std::vector<const char*>* defined = new std::vector<const char*>;
        static bool is_defined(const char* c);
        inline static int VERSION_MAJOR = 0;
        inline static int VERSION_MINOR = 2;

        inline static stack_frame* global_frame = new stack_frame();
        inline static std::vector<stack_frame*>* stack = nullptr;
        inline static std::stack<std::string>* errors = nullptr;

        inline static scanner* scan = nullptr;
        inline static bool has_init = false;
        inline static std::stack<int>* proc_num_ifs;
        inline static int num_procs_active = 0;

        // measure start time and end time
        inline static std::chrono::time_point<std::chrono::high_resolution_clock> start;
        inline static std::chrono::time_point<std::chrono::high_resolution_clock> end;
        inline static float time_taken = 0;
        inline static float avg_time_cumulative = 0;
        inline static float avg_time = 0;
        inline static int num_runs = 0;
        inline static data* last_run_data = nullptr;

        inline static bool in_proc_declaration = false;
        inline static proc_tokens* new_proc_tokens = nullptr;
        inline static proc_type_vec* types = nullptr;
        inline static long proc_stack_id = -1L;
        inline static int proc_type = NOTHING;
        inline static std::string proc_name;

        typedef std::vector<std::shared_ptr<token>> token_vec;

        inline static std::stack<std::queue<token_vec>> queue_stack = std::stack<std::queue<token_vec>>();

        static void init();

        // deprecated
        static char* const_char_convert(const char* input) {
            char* name = (char*)malloc(sizeof(char)*strlen(input));
            return strcpy(name, input);
        }

        static stack_frame* top_stack() {
            return stack->back();
        }

        static std::shared_ptr<token_group> evaluate_tokens(std::vector<std::shared_ptr<token>> tokens, int offset);
        static void input_loop();

        static int get_equal_index(const std::vector<std::shared_ptr<token>> &tokens);
        static std::vector<int> get_flags(const std::vector<std::shared_ptr<token>> &tokens);
        static void process_variable_declaration(const std::vector<std::shared_ptr<token>> &tokens);

        static void process_proc_declaration(std::vector<std::shared_ptr<token>> &tokens);
        static std::vector<std::shared_ptr<token>> clone_tokens(const std::vector<std::shared_ptr<token>> &tokens);
        static void end_proc_declaration();
        static void process_return(const token_vec & tokens, int offset);


        static bool set_literal(const std::vector<std::shared_ptr<token>> &tokens, data *d);

        static void process_variable_update(const std::vector<std::shared_ptr<token>> &tokens);


        static void process(const std::vector<std::shared_ptr<token>>& tokens);

        static void check_pop_stack(std::vector<std::shared_ptr<token>>& tokens);
        static void queue_input(std::string * input);

        static void run();
        static void read_from_file(const char* path);
        static void error(const std::string& err);
        static void print_errs();

        static void start_timer();
        static void end_timer();
        static void print_time();

    };
}
#endif //INTERPRETER_H
