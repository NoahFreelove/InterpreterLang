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
    stack = new std::vector<stack_frame*>();
    push_stackframe();
    global_frame = stack->front();
    std::filesystem::__cxx11::path cwd = std::filesystem::current_path();
    //global_frame->set(const_char_convert("WORKING_DIRECTORY"), new data(new std::string(cwd.string()), "string"));
    //global_frame->set(const_char_convert("VERSION"), new data(new float(VERSION_MAJOR + (VERSION_MINOR*0.01f)), "float"));

    has_init = true;
    if_block_statuses = new std::stack<bool>();
    if_results = new std::stack<bool>();

    defined->push_back("IMPLICIT_DOUBLE_TO_FLOAT");
    defined->push_back("IMPLICIT_UPCAST");
    errors = new std::stack<std::string>();
}

std::shared_ptr<token_group> lang::interpreter::evaluate_tokens(std::vector<std::shared_ptr<token>> tokens, int offset) {
    std::vector rest(tokens.begin() + offset, tokens.end());
    auto group = token_grouper::gen_group(rest);
    group_evaluator::eval_group(group);
    return group;
}

void lang::interpreter::input_loop() {
    auto* input = new std::string();
    if(scan == nullptr) {
        scan = new scanner();
    }
    while (true) {
        if(scan->in_multi_line() || scan->in_multi_comment() || scan->in_multi_string()) {
            std::cout << "... ";
        }
        else {
            std::cout << ">>> ";
        }
        std::getline(std::cin, *input);
        if (*input == "exit") {
            break;
        }
        process_input(input);
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

void lang::interpreter::process_variable_declaration(const std::vector<std::shared_ptr<token>> &tokens) {
    std::shared_ptr<token> type;
    const char* name;

    std::vector<int> flags = get_flags(tokens);
    bool persistent = false;
    bool is_final = false;
    int set_index = get_equal_index(tokens);

    if (tokens.size() >= 2) {
        if(tokens[1]->get_name() != IDENTIFIER || !tokens[0]->is_typeword()) {
            error("Invalid variable declaration");
            return;
        }
        name = tokens[1]->get_lexeme();
        type = tokens[0];
    }
    else {
        error("Not enough tokens for variable declaration");
        return;
    }

    // if the name ends with _old, it is invalid
    if(strstr(name, "_old")) {
        error("Invalid variable name, _old is reserved");
        return;
    }

    for (auto flag: flags) {
        if(flag == PERSISTENT)
            persistent = true;
        else if(flag == FINAL)
            is_final = true;
    }

    stack_frame* frame;
    if(persistent) {
        frame = global_frame;
    }
    else {
        frame = stack->back();
    }
    data* d = nullptr;
    switch (type->get_name()) {
        case INT_KEYW: {
            int* val = new int;
            *val = 0;
            d = new data(val, "int");
             frame->set(name, d);
             break;
            }
        case FLOAT_KEYW: {
            float *val = new float;
            *val = 0.0f;
            d = new data(val, "float");
            frame->set(name, d);
            break;
        }
        case DOUBLE_KEYW: {
            double *val = new double;
            *val = 0.0;
            d = new data(val, "double");
            frame->set(name, d);
            break;
        }
        case LONG_KEYW: {
            long *val = new long;
            *val = 0L;
            d = new data(val, "long");
            frame->set(name, d);
            break;
        }
        case STRING_KEYW: {
            d = new data(new std::string(), "string");
            frame->set(name, d);
            break;
        }
        case CHAR_KEYW: {
            char*val = new char;
            *val = '\0';
            d = new data(val, "char");
            frame->set(name, d);
            break;
        }
        case BOOL_KEYW: {
            bool* val = new bool;
            *val = false;
            d = new data(val, "bool");
            frame->set(name, d);
            break;
        }
        case ULONG64_KEYW: {
            auto* val = new unsigned long long;
            *val = 0;
            d = new data(val, "unsigned long long");
            frame->set(name, d);
            break;
        }
        default: {
            error("Invalid type word");
            return;
        }
    }
    if(set_index > 1) {
        std::vector concat_vec = {tokens[1]};
        // add everything after type index
        for(int i = set_index; i < tokens.size(); i++) {
            concat_vec.push_back(tokens[i]);
        }
        process_variable_update(concat_vec);
    }
    if(d && is_final) {
        d->set_final();
    }
        //std::cout << frame->get_data(name)->get() << std::endl;
}

bool lang::interpreter::set_literal(const std::vector<std::shared_ptr<token>> &tokens, data *d) {
    if(d) {
        auto group = evaluate_tokens(tokens, 2);
        if(group->type == ERROR) {
            error("error evaluating group");
            return false;
        }

        if(d->get_type() != token::type_to_char(group->type)) {
            // The default value for literals with decimals is a double which can be inconvinent if you have a float
            // as in most cases the float value can use the double value.
            if(d->get_type() == "float" && group->type == DOUBLE && is_defined("IMPLICIT_DOUBLE_TO_FLOAT")) {
                group->value = (float)std::any_cast<double>(group->value);
                group->type = FLOAT;
            }
            else if (is_defined("IMPLICIT_UPCAST")) {
                if(d->get_type() == "float" && (group->type == INT || group->type == LONG)) {
                    if(group->type == INT) {
                        group->value = (float)std::any_cast<int>(group->value);
                    }
                    else if (group->type == LONG) {
                        group->value = (float)std::any_cast<long>(group->value);
                    }
                    group->type = FLOAT;
                }
                else if(d->get_type() == "double" && (group->type == INT || group->type == LONG || group->type == FLOAT)) {
                    if(group->type == INT) {
                        group->value = (double)std::any_cast<int>(group->value);
                    }
                    else if (group->type == LONG) {
                        group->value = (double)std::any_cast<long>(group->value);
                    }
                    else if (group->type == FLOAT) {
                        group->value = (double)std::any_cast<float>(group->value);
                    }
                    group->type = DOUBLE;
                }
                else if(d->get_type() == "long" && group->type == INT) {
                    group->value = (long)std::any_cast<int>(group->value);
                    group->type = LONG;
                }
            }
            else {
                error("incompatible types, cannot set");
                return false;
            }
        }

        // token value is std::any, so we need to cast it to the correct type
        if(d->get_type() == "int") {
            //std::cout << "cast" << std::endl;
            d->set_value_int(std::any_cast<int>(group->value));
        }
        else if (d->get_type() == "float") {
            d->set_value_float(std::any_cast<float>(group->value));
        }
        else if (d->get_type() == "double") {
            d->set_value_double(std::any_cast<double>(group->value));
        }
        else if (d->get_type() == "long") {
            d->set_value_long(std::any_cast<long>(group->value));
        }
        else if (d->get_type() == "string") {
            // copy str
            //char* str = (char*)malloc(sizeof(char)*strlen(const_char_convert(std::any_cast<const char*>(tokens[2]->get_lexeme())))+1);
            d->set_value_string(std::any_cast<std::string>(group->value));
        }
        else if (d->get_type() == "char") {
            // value is going to be a string so we take the first character
            auto val = std::any_cast<std::string>(tokens[2]->get_lexeme());
            if(!val.empty()) {
                d->set_value_char(val[0]);
            }
            else {
                error("Invalid char value");
                return true;
            }
        }
        else if (d->get_type() == "bool") {
            if(group->type == TRUE) {
                d->set_value_bool(true);
            }
            else if(group->type == FALSE) {
                d->set_value_bool(false);
            }
            else {
                error("invalid bool type");
            }


        }
        else if (d->get_type() == "unsigned long long") {
            d->set_value_ulonglong(std::any_cast<unsigned long long>(tokens[2]->get_value()));
        }
        else {
            error("Invalid type");
            return true;
        }
    }
    return false;
}

void lang::interpreter::process_variable_update(const std::vector<std::shared_ptr<token>> &tokens) {
    if(tokens.size() < 3) {
        error("Not enough tokens for variable update");
        return;
    }
    if(tokens[0]->get_name() != IDENTIFIER || tokens[1]->get_name() != EQUAL) {
        error("Invalid variable update");
        return;
    }
    const char* name = tokens[0]->get_lexeme();
    data* d = resolve_variable(name);

    if(!d) {
        char c[strlen("Undefined variable with name: ")+ strlen(tokens[0]->get_lexeme())+1];
        strcpy(c, "Undefined variable with name: ");
        strcat(c,tokens[0]->get_lexeme());
        error(c);
        return;
    }
    if(tokens[2]->is_literal() || tokens[2]->get_name() == ID || tokens[2]->get_name() == MINUS || tokens[2]->get_name() == LEFT_PAREN || tokens[2]->get_name() == RIGHT_PAREN) {
        if (set_literal(tokens, d)) return;
    }
    else if(tokens[2]->get_name() == BYVAL && tokens.size() == 4) { // Doing byval has no effect but its technically valid
        if (set_literal(tokens, d)) return;
    }
    else if(tokens[2]->get_name() == INPUT) {
        auto str = new std::string();
        std::cout << "> ";
        std::getline(std::cin, *str);
        auto t = scan->scan_line(str);
        // set_literal expects first two tokens to be identifier =, so we prepend those
        t.insert(t.begin(), tokens[0]);
        t.insert(t.begin() + 1, tokens[1]);
        if (set_literal(t, d)) {
            delete str;
            return;
        }

        delete str;
    }
    else if (tokens[2]->get_name() == ID_GRAB && tokens[3]->get_name() == IDENTIFIER) {
        assign_variable(name, tokens[3]->get_lexeme());
    }
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
            std::cout << "No value" << std::endl;
        }
    }
}

void lang::interpreter::check_pop_stack(std::vector<std::shared_ptr<token>> &tokens) {
    if(tokens[tokens.size()-1]->get_name() == RIGHT_BRACE) {
        pop_stackframe();
        tokens.pop_back();
    }
}

void lang::interpreter::process_input( std::string *input) {
    if(scan == nullptr) {
        scan = new scanner();
    }

    init();
    while (!errors->empty())
        errors->pop();

    auto tokens = scan->scan_line(input);
    if(tokens.empty())
        return;

    // create seperate token vectors whenever a semicolon is found as a token. semicolon id = 13
    std::vector<std::vector<std::shared_ptr<token>>> token_vectors;
    std::vector<std::shared_ptr<token>> current_vector;
    for(auto token : tokens) {
        if(token->get_name() == SEMICOLON) {
            token_vectors.push_back(current_vector);
            current_vector.clear();
        }
        else {
            current_vector.push_back(token);
        }
    }
    if(!current_vector.empty()) {
        token_vectors.push_back(current_vector);
    }

    for (auto &token_vector : token_vectors) {

        if(!if_block_statuses->empty()) {
            if(token_vector[0]->get_name() == END_IF) {
                if_block_statuses->pop();
                check_pop_stack(tokens);
                continue;
            }
            if(if_block_statuses->top() == false)
                continue;
        }
        if(token_vector[0]->get_name() == LEFT_BRACE) {
            push_stackframe();
            token_vector.erase(token_vector.begin());
        }
        if (token_vector[0]->is_typeword()) {
            process_variable_declaration(token_vector);
            check_pop_stack(tokens);
            continue;
        }
        if(token_vector[0]->is_control_flow()) {
            control_flow_runner::process_control_flow(tokens);
            check_pop_stack(tokens);
            continue;
        }
        if(token_vector[0]->is_builtin()) {
            run_builtins(token_vector);
            check_pop_stack(tokens);
            continue;
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
                    check_pop_stack(tokens);
                    continue;
                }
            }

        }
        process(token_vector);
        check_pop_stack(tokens);
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
        process_input(&str);
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
