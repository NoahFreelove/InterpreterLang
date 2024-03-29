#include "interpreter.h"

#include <fstream>
#include <stack>
#include <cstring>
#include <utility>
#include <bits/fs_fwd.h>
#include <bits/fs_path.h>

#include "evaluator/group_evaluator.h"
#include "tokenizer/tokens.h"

#include "memory/stack_manager.h"
#include "memory/stack_frame.h"
#include "memory/type_registry.h"

#include "executors/built_in_runner.h"
#include "executors/control_flow_runner.h"
#include "executors/var_setter.h"
#include "executors/loop_executor.h"
#include "memory/macro.h"
bool lang::interpreter::is_defined(const char *c) {
    for (const char* name : *defined) {
        if(strcmp(name, c) == 0) {
            return true;
        }
    }
    return false;
}

void lang::interpreter::exit(int code) {
    std::exit(code);
}

void lang::interpreter::init() {
    if(has_init)
        return;
    if(scan == nullptr) {
        scan = new scanner();
    }
    stack = new std::vector<stack_frame*>();
    macros = new std::unordered_map<std::string, macro*>();
    push_stackframe();
    global_frame = stack->front();
    std::filesystem::__cxx11::path cwd = std::filesystem::current_path();
    global_frame->set("WORKING_DIRECTORY", new data(new std::string(cwd.string()), "string"));
    global_frame->set("VERSION", new data(new float(VERSION_MAJOR + (VERSION_MINOR*0.01f)), "float", false, true));

    has_init = true;
    proc_num_ifs = new std::stack<int>();
    //if_results = new std::stack<bool>();

    defined->push_back("IMPLICIT_DOUBLE_TO_FLOAT");
    defined->push_back("IMPLICIT_UPCAST");
    defined->push_back("PROC_OVERLOAD");
    defined->push_back("PROC_REDEFINITION");
    errors = new std::stack<std::string>();
    read_from_file("about.lang");
    read_from_file("stdlib.lang");

    type_registry::add_type(INT, "int");
    type_registry::add_type(FLOAT, "float");
    type_registry::add_type(DOUBLE, "double");
    type_registry::add_type(LONG, "long");
    type_registry::add_type(STRING, "string");
    type_registry::add_type(ULONG64, "ulong64");
    type_registry::add_type(BOOL_KEYW, "bool");
    type_registry::add_type(NOTHING_TYPE, "nothing");
    type_registry::add_type(TRUE, "true");
    type_registry::add_type(FALSE, "false");
}

std::shared_ptr<token_group> lang::interpreter::evaluate_tokens(std::vector<std::shared_ptr<token>> tokens, int offset) {
    std::vector rest(tokens.begin() + offset, tokens.end());
    auto group = token_grouper::gen_group(rest);
    //group->print_group();
    group_evaluator::eval_group(group);
    return group;
}

using token_element = std::variant<std::shared_ptr<token>, std::shared_ptr<token_group>>;
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

data * lang::interpreter::recursive_array_simplification(std::shared_ptr<token_group>& group) {
    group_evaluator::recursive_replace(group, false);
    if(group->tokens.size() == 1) {
        if(!group_evaluator::is_group(*group->tokens[0])) {
            auto t = *std::get<std::shared_ptr<token>>(*group->tokens[0]);
            if(t.get_name() == DATA) {
                return std::any_cast<data*>(t.get_value());
            }
        }
        else {
            error("Array simplification did not evaluate properly");
        }
    }
    else {
        //group->print_group();
        recursive_array_simplification(group);

    }
    return nullptr;
}

bool lang::interpreter::check_array_mutation(const token_vec &input) {
    token_vec copy = token_vec(input);
    token_vec before_eq;
    int i = 0;
    bool found_eq = false;
    bool in_bracket = false;
    for (; i+1 < input.size(); ++i) {
        if(input[i+1]->get_name() == EQUAL) {
            found_eq = true;
            if(!input[i]->is_arithmetic()) {
                before_eq.push_back(input[i]);
            }
            else {
                i--;
            }
            break;
        }
        if(input[i]->get_name() == LEFT_BRACKET) {
            if(in_bracket) {
                error("Cannot nested subscript");
                return false;
            }
            in_bracket = true;
        }
        else if(input[i]->get_name() == RIGHT_BRACKET) {
            if(!in_bracket) {
                error("Cannot close unopened subscript");
                return false;
            }
            in_bracket = false;
        }
        
        before_eq.push_back(input[i]);
    }


    if(!found_eq)
        return false;

    copy.erase(copy.begin(), copy.begin()+i+1);

    /*for (auto& t : before_eq) {
        std::cout << *t << std::endl;
    }*/

    if(copy.empty())
        return false;

    auto group = std::make_shared<token_group>();
    for (auto& tok : before_eq) {
        group->tokens.push_back(std::make_shared<token_element>(tok));
    }
    data* dat = recursive_array_simplification(group);
    if(!dat) {
        error("Array simplification failed");
        return false;
    }

    bool is_arithmetic = copy[0]->is_arithmetic();

    if(is_arithmetic) {
        copy.erase(copy.begin() + 1);
        auto t = token_representation_from_data(dat);
        copy.insert(copy.begin(), t);
    }
    else {
        copy.erase(copy.begin());
    }

    set_literal(copy, dat, 0);



    return true;
}

void lang::interpreter::input_loop() {
    init();
    trigger_run();
    auto* input = new std::string();
    if(scan == nullptr) {
        scan = new scanner();
    }
    while (true) {
        if(scan->in_multi_line() || scan->in_multi_comment() || scan->in_multi_string() || loop_executor::in_loop_declaration) {
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
        trigger_run();
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

std::vector<std::shared_ptr<token>> lang::interpreter::clone_tokens(const std::vector<std::shared_ptr<token>> &tokens) {
    std::vector<std::shared_ptr<token>> cloned = std::vector<std::shared_ptr<token>>();
    cloned.reserve(tokens.size());
    for(const auto& t : tokens) {
        auto new_token = std::make_shared<token>(t.get());
        cloned.push_back(new_token);
    }
    return cloned;
}

void lang::interpreter::process(std::vector<std::shared_ptr<token>> tokens) {
    std::shared_ptr<token_group> group = token_grouper::gen_group(std::move(tokens));

    std::vector<std::shared_ptr<token>> output;
    group_evaluator::flatten(output, group);

    group_evaluator::eval_group(group);
    if(group->type == ERROR) {
        error("Could not process input");
    }
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
    if(tokens.empty()) {
        //error("tokens empty cant check pop stack");
        return;
    }
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
    queue_lines(queue, GENERAL_INPUT);
}

void lang::interpreter::run() {
    init();
    if(queue_stack.empty())
        return;
    while (!queue_stack.top().empty()) {
        //std::cout << "SIZE OF ALL QUEUES: " << queue_stack.size() << std::endl;
        auto queue = queue_stack.top();
        current_quality = queue_qualties.top();
        queue_qualties.pop();
        queue_stack.pop();

        while (!queue.empty()) {
            //std::cout << "command" << std::endl;
            token_vec token_vector = queue.front();
            if(token_vector.empty()) {
                queue.pop();
                continue;
            }
            token_vec copy = token_vec();
            for (auto& t : token_vector) {
                copy.push_back(t);
            }
            queue.pop();

            //std::cout << "SIZE: " << control_flow_runner::blockStack.size() << std::endl;
            if(in_proc_declaration) {
                if(!copy.empty()) {
                    if(copy[0]->get_name() == END_PROC) {
                        proc_manager::end_proc_declaration();
                    }
                    else {
                        if(proc_manager::new_proc_tokens) {
                            proc_manager::new_proc_tokens->push_back(clone_tokens(copy));
                        }
                        else {
                            error("Current procedure does not exist.");
                        }
                    }
                }
                continue;
            }
            if(loop_executor::in_loop_declaration) {
                if(!copy.empty()) {
                    if(copy[0]->is_loop()) {
                        loop_executor::loop_declaration_nest++;
                    }

                    if(copy[0]->get_name() == END_LOOP && loop_executor::loop_declaration_nest == 1) {
                        loop_executor::end_loop_declaration();
                        continue;
                    }
                    if(copy[0]->get_name() == END_LOOP) {
                        loop_executor::loop_declaration_nest--;
                    }

                    if(loop_executor::current_loop) {
                        loop_executor::current_loop->loop_lines->push_back(clone_tokens(copy));
                        continue;
                    }
                    else {
                        error("Current loop does not exist. Call end_loop to fix this.");
                    }
                }
            }
            //std::cout << id_to_name(copy[0]->get_name()) << std::endl;
            /*if(!proc_num_ifs->empty())
                std::cout<< "numifs:" << proc_num_ifs->top() << std::endl;
                */

            if(!control_flow_runner::blockStack.empty()) {
                if(copy[0]->get_name() == END_IF) {
                    //std::cout << "ENDIF " <<std::endl;
                    if(!proc_num_ifs->empty()) {
                        if(proc_num_ifs->top() == 0 && current_quality == PROC_INPUT) {
                            // look at control_flow_runner's to-do to fix this
                            error("No more ifs to pop in proc!");
                           continue;
                        }

                        int num = proc_num_ifs->top() - 1;
                        proc_num_ifs->pop();
                        proc_num_ifs->push(num);

                        if(loop_executor::current_loop_index > -1) {
                            loop_executor::active_loops[loop_executor::current_loop_index]->loop_ifs--;
                        }
                    }
                    if(proc_num_ifs->empty() && current_quality == PROC_INPUT) {
                        std::cout << "Procnumifs empty, adding to stack and ignoring handleEndIf. " <<std::endl;
                        proc_num_ifs->push(0);
                        continue;
                    }

                    control_flow_runner::handleEndIf();

                    check_pop_stack(copy);
                    continue;
                }
            }else if(copy[0]->get_name() == END_IF) {
                error("Error: Unmatched 'end if' found");
            }
            if(copy[0]->is_control_flow()) {
                control_flow_runner::process_control_flow(copy);
                check_pop_stack(copy);
                continue;
            }
            if(!control_flow_runner::shouldExecuteCurrentBlock()) {
                //std::cout << "IN INVALID BLOCK" << std::endl;
                continue;
            }
            if(loop_executor::in_loop()) {
                if(copy.size() == 1) {
                    if(copy[0]->get_name() == BREAK) {
                        loop_executor::trigger_break = true;
                        if(loop_executor::current_loop_index > -1) {
                            auto* target = loop_executor::active_loops[loop_executor::current_loop_index];
                            for (int i = 0; i < target->loop_ifs; ++i) {
                                control_flow_runner::handleEndIf();
                            }
                            target->loop_ifs = 0;
                        }
                        break;
                    }
                    else if(copy[0]->get_name() == CONTINUE) {
                        if(loop_executor::current_loop_index > -1) {
                            auto* target = loop_executor::active_loops[loop_executor::current_loop_index];
                            for (int i = 0; i < target->loop_ifs; ++i) {
                                control_flow_runner::handleEndIf();
                            }
                            target->loop_ifs = 0;
                        }
                        break;
                    }
                }
            }

            if(copy[0]->get_name() == RETURN) {
                /*while (!queue.empty()) {
                    queue.pop();
                }
                */

                proc_manager::process_return(copy, 1);
                //std::cout << "SIZE: " <<  queue_stack.size() <<std::endl;
                //queue_stack.pop();
                if(num_procs_active > 0) {
                    num_procs_active--;
                }

                return;
            }

            if(copy[0]->get_name() == LEFT_BRACE) {
                push_stackframe();
                copy.erase(copy.begin());
            }
            if (copy[0]->is_typeword()) {
                process_variable_declaration(copy);
                check_pop_stack(copy);
                continue;
            }
            if(copy[0]->is_builtin()) {
                run_builtins(copy);
                //check_pop_stack(copy);
                continue;
            }
            if(copy[0]->get_name() == PROC_KEYW) {
                if(control_flow_runner::blockStack.empty() || loop_executor::in_loop()) {
                    proc_manager::process_proc_declaration(copy);
                    continue;
                }
                else {
                    error("Cannot declare procedure in any kind of if-block or loop");
                    continue;
                }
            }
            if(copy[0]->is_loop()) {
                loop_executor::process_loop(copy);
                check_pop_stack(copy);
                continue;
            }

            if(copy.size() >= 3) {
                if(copy[0]->get_name() == MACRO) {
                    auto* result = macro::gen_macro(copy);
                    if(result) {
                        if(macros->find(std::string(copy[1]->get_lexeme())) != macros->end()) {
                            error("Attempted redefinition of macro '" + std::string(copy[1]->get_lexeme()) + "'");
                            delete result;
                        }
                        else
                            macros->insert({std::string(copy[1]->get_lexeme()), result});
                    }
                    else {
                        error("Error defining macro");
                    }
                    continue;
                }
            }

            if(copy.size() >=2) {
                if(copy[0]->get_name() == IDENTIFIER) {
                    if(copy.size() >= 3) {
                        if(copy[1]->is_arithmetic() && copy[2]->get_name() == EQUAL) {
                            arithmetic_evaluator::convert_op_eq_to_op(copy);
                        }
                    }
                    if(copy.size() == 3) {
                        if(copy[1]->get_name() == PLUS && copy[2]->get_name() == PLUS) {
                            arithmetic_evaluator::convert_inc_to_op(copy);
                        }
                        else if(copy[1]->get_name() == MINUS && copy[2]->get_name() == MINUS) {
                            arithmetic_evaluator::convert_dec_to_op(copy);
                        }
                    }

                    if(copy[1]->get_name() == EQUAL) {
                        process_variable_update(copy);
                        check_pop_stack(copy);
                        continue;
                    }
                    if(copy[1]->get_name() == LEFT_BRACKET) {
                        if(check_array_mutation(copy)) {
                            check_pop_stack(copy);
                            continue;
                        }
                    }
                }
            }
            process(copy);
            check_pop_stack(copy);
        }
        if(queue_stack.empty()) {
            return;
        }
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
        trigger_run();
    }
    file.close();
}

void lang::interpreter::error(const std::string& err) {
    errors->push(err);
}

void lang::interpreter::print_errs() {
    if(errors->empty()) {
        return;
    }
    bool sizeis1 = errors->size() == 1;
    std::cerr << "Error";
    if(errors->size() == 1) {
         std::cerr << ": "<< std::endl;
    }
    else {
         std::cerr << "s: "<< std::endl;
    }
    while (!errors->empty()) {
        if(errors->size() == 1 && !sizeis1) {
            std::cerr << "The following error likely caused any errors above:" << std::endl;
            std::cerr << "-> -> -> " << errors->top() << std::endl;
        }
        else
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

void lang::interpreter::trigger_run() {
    run();
    if(!errors->empty()) {
        print_errs();
        while (!errors->empty()) {
            errors->pop();
        }
    }
}
