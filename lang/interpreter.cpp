#include "interpreter.h"

#include <fstream>
#include <stack>
#include <cstring>
#include <bits/fs_fwd.h>
#include <bits/fs_path.h>

#include "evaluator/group_evaluator.h"
#include "memory/stack_frame.h"
#include "tokenizer/tokens.h"
#include "executors/built_in_runner.h"
#include "executors/control_flow_runner.h"
#include "memory/stack_manager.h"
#include "executors/var_setter.h"
#include "executors/loop_executor.h"
bool lang::interpreter::is_defined(const char *c) {
    for (const char* name : *defined) {
        if(strcmp(name, c) == 0) {
            return true;
        }
    }
    return false;
}

void lang::interpreter::init() {
    if(has_init)
        return;
    if(scan == nullptr) {
        scan = new scanner();
    }
    stack = new std::vector<stack_frame*>();
    push_stackframe();
    global_frame = stack->front();
    std::filesystem::__cxx11::path cwd = std::filesystem::current_path();
    global_frame->set("WORKING_DIRECTORY", new data(new std::string(cwd.string()), "string"));
    global_frame->set("VERSION", new data(new float(VERSION_MAJOR + (VERSION_MINOR*0.01f)), "float"));

    has_init = true;
    proc_num_ifs = new std::stack<int>();
    //if_results = new std::stack<bool>();

    defined->push_back("IMPLICIT_DOUBLE_TO_FLOAT");
    defined->push_back("IMPLICIT_UPCAST");
    defined->push_back("PROC_OVERLOAD");
    defined->push_back("PROC_REDEFINITION");
    errors = new std::stack<std::string>();
    read_from_file("about.lang");
}

std::shared_ptr<token_group> lang::interpreter::evaluate_tokens(std::vector<std::shared_ptr<token>> tokens, int offset) {
    std::vector rest(tokens.begin() + offset, tokens.end());
    auto group = token_grouper::gen_group(rest);
    group_evaluator::eval_group(group);
    return group;
}

void lang::interpreter::input_loop() {
    init();
    run();
    auto* input = new std::string();
    if(scan == nullptr) {
        scan = new scanner();
    }
    while (true) {
        if(scan->in_multi_line() || scan->in_multi_comment() || scan->in_multi_string()) {
            std::cout << "... ";
        }
        else if (in_proc_declaration) {
            std::cout << ">>>> ";
        }
        else {
            std::cout << ">>> ";
        }
        std::getline(std::cin, *input);
        if (*input == "exit") {
            break;
        }
        start_timer();
        queue_input(input);
        run();
        end_timer();
    }
}

int lang::interpreter::get_equal_index(const std::vector<std::shared_ptr<token>> &tokens) {
    for (int i = 0; i < tokens.size(); ++i) {
        if(tokens[i]->get_name() == EQUAL)
            return i;
    }
    return -1;
}

std::vector<int> lang::interpreter::get_flags(const std::vector<std::shared_ptr<token>> &tokens) {
    std::vector<int> flags;
    // Tokens 0 and 1 are type and identifier
    for (int i = 2; i < tokens.size(); ++i) {
        if(tokens[i]->get_name() == EQUAL) {
            return flags;
        }
        if(tokens[i]->get_name() == PERSISTENT || tokens[i]->get_name() == FINAL) {
            flags.push_back(tokens[i]->get_name());
        }
        else {
            error("illegal flag: " + id_to_name( tokens[i]->get_name()));
            return flags;
        }
    }
    return flags;
}
std::vector<std::shared_ptr<token>> lang::interpreter::clone_tokens(const std::vector<std::shared_ptr<token>> &tokens) {
    std::vector<std::shared_ptr<token>> cloned = std::vector<std::shared_ptr<token>>();
    for(const auto& t : tokens) {
        auto new_token = std::make_shared<token>(t->get_name(), t->get_lexeme(), t->get_line(), t->get_value());
        cloned.push_back(new_token);
    }
    return cloned;
}

void lang::interpreter::process(const std::vector<std::shared_ptr<token>>& tokens) {
    /*for (const std::shared_ptr<token> t : tokens) {
        std::cout << *t << std::endl;
    }
    return;*/
    std::shared_ptr<token_group> group = token_grouper::gen_group(tokens);
    //group->print_group();
    group_evaluator::eval_group(group);
    if(group->type == ERROR) {
        error("Could not process input");
    }
    //group->print_group();
    if(group->value.has_value()) {
        if(group->type == INT) {
            std::cout << std::any_cast<int>(group->value) << std::endl;
        }
        else if(group->type == FLOAT) {
            std::cout << std::any_cast<float>(group->value) << std::endl;
        }
        else if(group->type == DOUBLE) {
            std::cout << std::any_cast<double>(group->value) << std::endl;
        }
        else if(group->type == LONG) {
            std::cout << std::any_cast<long>(group->value) << std::endl;
        }
        else if(group->type == STRING) {
            std::cout << '"' <<std::any_cast<std::string>(group->value) << '"' << std::endl;
        }
        else if(group->type == ULONG64) {
            std::cout << std::any_cast<unsigned long long>(group->value) << std::endl;
        }
        else if (group->type == TRUE) {
            std::cout << "true" << std::endl;
        }
        else if (group->type == FALSE) {
            std::cout << "false" << std::endl;
        }
        else {
            //std::cout << "No value" << std::endl;
        }
    }
}

void lang::interpreter::check_pop_stack(std::vector<std::shared_ptr<token>> &tokens) {
    if(tokens[tokens.size()-1]->get_name() == RIGHT_BRACE) {
        pop_stackframe();
        tokens.pop_back();
    }
}

void lang::interpreter::queue_input(std::string *input)  {
    init();
    auto tokens = scan->scan_line(input);
    if(tokens.empty())
        return;
    auto queue = std::queue<token_vec>();
    // create seperate token vectors whenever a semicolon is found as a token. semicolon id = 13
    token_vec current_vector = token_vec();
    for(const auto& t : tokens) {
        if(t->get_name() == SEMICOLON) {
            queue.emplace(current_vector);
            current_vector.clear();
        }
        else {
            current_vector.push_back(t);
        }
    }
    if(!current_vector.empty()) {
        queue.push(current_vector);
    }
    queue_stack.push(queue);
}

void lang::interpreter::run() {
    init();
    while (!errors->empty())
        errors->pop();
    if(queue_stack.empty())
        return;

    while (!queue_stack.top().empty()) {
        auto queue = queue_stack.top();
        queue_stack.pop();

        while (!queue.empty()) {
            token_vec token_vector = queue.front();
            token_vec copy = token_vec();
            for (auto& t : token_vector) {
                copy.push_back(t);
            }
            queue.pop();
            //std::cout << "SIZE: " << control_flow_runner::blockStack.size() << std::endl;

            if(in_proc_declaration) {
                if(!token_vector.empty()) {
                    if(token_vector[0]->get_name() == END_PROC) {
                        proc_manager::end_proc_declaration();
                    }
                    else {
                        if(proc_manager::new_proc_tokens) {
                            proc_manager::new_proc_tokens->push_back(clone_tokens(token_vector));
                        }
                        else {
                            error("Current procedure does not exist.");
                        }
                    }
                }
                continue;
            }

            if(!control_flow_runner::blockStack.empty()) {
                if(token_vector[0]->get_name() == END_IF) {
                    //std::cout << "ENDIF " <<std::endl;
                    if(!proc_num_ifs->empty()) {
                        if(proc_num_ifs->top() == 0) {
                            error("No more ifs to pop in proc!");
                            return;
                        }

                        int num = proc_num_ifs->top() - 1;
                        proc_num_ifs->pop();
                        proc_num_ifs->push(num);
                    }

                    control_flow_runner::handleEndIf();

                    check_pop_stack(copy);
                    continue;
                }
            }
            if(token_vector[0]->is_control_flow()) {
                control_flow_runner::process_control_flow(token_vector);
                check_pop_stack(copy);
                continue;
            }

            if(!control_flow_runner::shouldExecuteCurrentBlock())
                continue;

            if(token_vector[0]->get_name() == RETURN) {
                while (!queue.empty()) {
                    queue.pop();
                }
                proc_manager::process_return(token_vector, 1);

                continue;
            }

            if(token_vector[0]->get_name() == LEFT_BRACE) {
                push_stackframe();
                token_vector.erase(token_vector.begin());
            }
            if (token_vector[0]->is_typeword()) {
                process_variable_declaration(token_vector);
                check_pop_stack(copy);
                continue;
            }
            if(token_vector[0]->is_builtin()) {
                run_builtins(token_vector);
                check_pop_stack(copy);
                continue;
            }
            if(token_vector[0]->get_name() == PROC_KEYW) {
                if(control_flow_runner::blockStack.empty()) {
                    proc_manager::process_proc_declaration(token_vector);
                    continue;
                }
                else {
                    error("Cannot declare procedure in any kind of if-block");
                    continue;
                }
            }

            if(token_vector.size() >=2) {
                if(token_vector[0]->get_name() == IDENTIFIER) {
                    if(token_vector.size() >= 3) {
                        if(token_vector[1]->is_arithmetic() && token_vector[2]->get_name() == EQUAL) {
                            arithmetic_evaluator::convert_op_eq_to_op(token_vector);
                        }
                    }
                    if(token_vector.size() == 3) {
                        if(token_vector[1]->get_name() == PLUS && token_vector[2]->get_name() == PLUS) {
                            arithmetic_evaluator::convert_inc_to_op(token_vector);
                        }
                        else if(token_vector[1]->get_name() == MINUS && token_vector[2]->get_name() == MINUS) {
                            arithmetic_evaluator::convert_dec_to_op(token_vector);
                        }
                    }

                    if(token_vector[1]->get_name() == EQUAL) {
                        process_variable_update(token_vector);
                        check_pop_stack(copy);
                        continue;
                    }
                }

            }
            process(token_vector);
            check_pop_stack(copy);
        }
        if(num_procs_active > 0) {
            num_procs_active--;
        }
        if(queue_stack.empty()) {
            num_procs_active = 0;
            break;
        }
    }
    if(!errors->empty()) {
        print_errs();
    }
}

void lang::interpreter::read_from_file(const char *path) {
    // read from file path. Read line by line and call process_input on each line.
    std::ifstream file(path);
    std::string str;
    if(!file.is_open()) {
        std::cerr << "Error: File not found" << std::endl;
        return;
    }
    if(!file.good()) {
        std::cerr << "Error: File empty" << std::endl;
        return;
    }
    while(std::getline(file, str)) {
        queue_input(&str);
        run();
    }
    file.close();
}

void lang::interpreter::error(const std::string& err) {
    errors->push(err);
}

void lang::interpreter::print_errs() {
    std::cerr << "Error";
    if(errors->size() == 1) {
         std::cerr << ": "<< std::endl;
    }
    else {
         std::cerr << "s: "<< std::endl;
    }
    while (!errors->empty()) {
        std::cerr << "-> " << errors->top() << std::endl;
        errors->pop();
    }
}

void lang::interpreter::start_timer() {
    start = std::chrono::high_resolution_clock::now();
}

void lang::interpreter::end_timer() {
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = end - start;
    time_taken = duration.count();
    avg_time_cumulative += time_taken;
    num_runs++;

    avg_time = avg_time_cumulative/(float)num_runs;

    if(last_run_data == nullptr) {
        last_run_data = new data(new float(time_taken), "float");
        global_frame->set("INTERPRETER_TIME", last_run_data);
    }
    last_run_data->set_value_float(time_taken);
}

void lang::interpreter::print_time() {
    std::cout << "Last run time: " << time_taken << "s" << std::endl;
    std::cout << "Average time: " << avg_time << "s" << std::endl;
}
