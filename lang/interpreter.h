#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <chrono>
#include <stack>
#include <queue>

#include "tokenizer/scanner.h"
#include "memory/memory.h"
#include "tokenizer/token_group.h"
#include "memory/proc_manager.h"
#define GENERAL_INPUT 0
#define PROC_INPUT 1
#define LOOP_INPUT 2
class macro;

namespace lang {
    class scanner;

    class interpreter {
    public:
        inline static const int max_depth = 100;
        inline static std::vector<const char*>* defined = new std::vector<const char*>;
        static bool is_defined(const char* c);

        static void exit(int code = 0);

        inline static int VERSION_MAJOR = 0;
        inline static int VERSION_MINOR = 40;

        inline static stack_frame* global_frame = new stack_frame();
        inline static std::unordered_map<std::string, macro*>* macros = nullptr;
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


        typedef std::vector<std::shared_ptr<token>> token_vec;
    private:
        inline static std::stack<std::queue<token_vec>> queue_stack = std::stack<std::queue<token_vec>>();
        inline static std::stack<int> queue_qualties = std::stack<int>();
    public:
        static void queue_lines(const std::queue<token_vec> &lines, int quality) {
            //std::cout << "QUEUING WITH QUALITY: " << quality << std::endl;;
            queue_stack.push(lines);
            queue_qualties.push(quality);
        }
        inline static int current_quality = GENERAL_INPUT;
        static void init();

        static stack_frame* top_stack() {
            return stack->back();
        }

        static std::shared_ptr<token_group> evaluate_tokens(std::vector<std::shared_ptr<token>> tokens, int offset);

        static data* recursive_array_simplification(std::shared_ptr<token_group>& group);
        static bool check_array_mutation(const token_vec &copy);

        static void input_loop();

        static int get_equal_index(const std::vector<std::shared_ptr<token>> &tokens);
        static std::vector<std::shared_ptr<token>> clone_tokens(const std::vector<std::shared_ptr<token>> &tokens);
        static void process(std::vector<std::shared_ptr<token>>& tokens);

        static void check_pop_stack(std::vector<std::shared_ptr<token>>& tokens);
        static void queue_input(std::string * input);


        static void read_from_file(const char* path);
        static void error(const std::string& err);
        static void print_errs();

        static void start_timer();
        static void end_timer();
        static void print_time();
        static void trigger_run();
    private:
        static void run();
    };
}
#endif //INTERPRETER_H
