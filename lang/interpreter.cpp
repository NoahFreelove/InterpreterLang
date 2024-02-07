#include "interpreter.h"

#include <fstream>
#include <stack>
#include <string.h>

#include "evaluator/group_evaluator.h"
#include "memory/stack_frame.h"
#include "tokenizer/tokens.h"

void lang::interpreter::input_loop() {
    auto* input = new std::string();
    auto scan = scanner();
    while (true) {
        if(scan.in_multi_line() || scan.in_multi_comment() || scan.in_multi_string()) {
            std::cout << "... ";
        }
        else {
            std::cout << ">>> ";
        }
        std::getline(std::cin, *input);
        if (*input == "exit") {
            break;
        }
        process_input(scan, input);
    }
}

void lang::interpreter::process_variable_declaration(const std::vector<token*> &tokens) {
    if (tokens.size() >= 3) {
        if(tokens[1]->get_name() != IDENTIFIER || tokens[2]->get_name() != AS || !tokens[3]->is_typeword()) {
            error("Invalid variable declaration");
            return;
        }
        // if the name ends with _old, it is invalid
        if(strstr(tokens[1]->get_lexeme(), "_old")) {
            error("Invalid variable name, _old is reserved");
            return;
        }

        auto* frame = stack->top();
        char* name = const_char_convert(tokens[1]->get_lexeme());

        switch (tokens[3]->get_name()) {
            case INT_KEYW: {
                int* val = new int;
                *val = 0;
                frame->set(name, new data(val, "int"));
                break;
            }
            case FLOAT_KEYW: {
                float *val = new float;
                *val = 0.0;
                frame->set(name, new data(val, "float"));
                break;
            }
            case DOUBLE_KEYW: {
                double *val = new double;
                *val = 0.0;
                frame->set(name, new data(val, "double"));
                break;
            }
            case LONG_KEYW: {
                long *val = new long;
                *val = 0;
                frame->set(name, new data(val, "long"));
                break;
            }
            case STRING_KEYW: {
                char** val = new char*;
                *val = (char*)malloc(sizeof(char)*1);
                frame->set(name, new data(val, "string"));
                break;
            }
            case CHAR_KEYW: {
                char*val = new char;
                *val = '\0';
                frame->set(name, new data(val, "char"));
                break;
            }
            case BOOL_KEYW: {
                bool* val = new bool;
                *val = false;
                frame->set(name, new data(val, "bool"));
                break;
            }
            case ULONG64_KEYW: {
                auto* val = new unsigned long long;
                *val = 0;
                frame->set(name, new data(val, "unsigned long long"));
                break;
            }
            default: {
                error("Invalid type word");
                return;
            }
        }
        //std::cout << frame->get_data(name)->get() << std::endl;
    }
    else {
        error("Not enough tokens for variable declaration");
        return;
    }
}

bool lang::interpreter::set_literal(const std::vector<token *> &tokens, data *d) {
    if(d) {
        // token value is std::any, so we need to cast it to the correct type
        if(d->get_type() == "int") {
            std::vector<token*> rest(tokens.begin() + 2, tokens.end());
            for (auto& token : rest ){
                //std::cout << id_to_name(token->get_name()) << std::endl;
            }
            token_group* group = token_grouper::recursive_group(rest);
            group_evaluator::eval_group(group);
            d->set_value_int(std::any_cast<int>(group->value));
        }
        else if (d->get_type() == "float") {
            d->set_value_float(std::any_cast<float>(tokens[2]->get_value()));
        }
        else if (d->get_type() == "double") {
            d->set_value_double(std::any_cast<double>(tokens[2]->get_value()));
        }
        else if (d->get_type() == "long") {
            d->set_value_long(std::any_cast<long>(tokens[2]->get_value()));
        }
        else if (d->get_type() == "string") {
            d->set_value_string(const_char_convert(std::any_cast<const char*>(tokens[2]->get_lexeme())));
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
            std::cout << "Setting bool" << std::endl;
            std::cout<< tokens[2]->get_name() << std::endl;
            if(tokens[2]->get_name() == FALSE)
                d->set_value_bool(false);
            else if(tokens[2]->get_name() == TRUE)
                d->set_value_bool(true);
            else {
                error("invalid bool value");
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

void lang::interpreter::process_variable_update(const std::vector<token *> &tokens) {
    if(tokens.size() < 3) {
        error("Not enough tokens for variable update");
        return;
    }
    if(tokens[0]->get_name() != IDENTIFIER || tokens[1]->get_name() != EQUAL) {
        error("Invalid variable update");
        return;
    }
    auto* frame = stack->top();
    char* name = const_char_convert(tokens[0]->get_lexeme());
    data* d = frame->get_data(name);
    if(tokens[2]->is_literal() || tokens[2]->get_name() == LEFT_PAREN || tokens[2]->get_name() == RIGHT_PAREN) {
        if (set_literal(tokens, d)) return;
    }
    else if (tokens[2]->get_name() == ID_GRAB && tokens[3]->get_name() == IDENTIFIER) {
        frame->assign(name, const_char_convert(tokens[3]->get_lexeme()));
    }
}

void lang::interpreter::print(const std::vector<token *>& tokens) {
    if(tokens.size() < 2) {
        error("Not enough tokens for print statement");
        return;
    }
    if(tokens[1]->get_name()== IDENTIFIER) {
        data* d = stack->top()->get_data(const_char_convert(tokens[1]->get_lexeme()));
        if(d) {
            std::cout << d->to_string() << std::endl;
        }
    }
    else if(tokens[1]->is_literal_non_id()) {
        std::cout << tokens[1]->get_lexeme() << std::endl;
    }
    else {
        std::cout << id_to_name(tokens[1]->get_name()) << std::endl;
    }
}

void lang::interpreter::process(const std::vector<token *>& tokens) {
    for (auto token : tokens) {
        std::cout << *token << std::endl;
    }
}

void lang::interpreter::process_input(scanner& scan, std::string *input) {
    if(stack == nullptr) {
        stack = new std::stack<stack_frame*>();
        stack->push(new stack_frame());
    }

    auto tokens = scan.scan_line(input);
    if(tokens.empty())
        return;

    // create seperate token vectors whenever a semicolon is found as a token. semicolon id = 13
    std::vector<std::vector<token*>> token_vectors;
    std::vector<token*> current_vector;
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
        if (token_vector[0]->is_keyword()) {
            if (token_vector[0]->get_name() == VAR) {
                process_variable_declaration(token_vector);
            }
        }
        if(token_vector[0]->is_builtin()) {
            if (token_vector[0]->get_name() == PRINT) {
                print(token_vector);
            }
            else if (token_vector[0]->get_name() == DUMP) {
                stack->top()->dump_memory();
                return;
            }
        }
        if(token_vector.size() >=2) {
            if(token_vector[0]->get_name() == IDENTIFIER && token_vector[1]->get_name() == EQUAL) {
                process_variable_update(token_vector);
            }
        }
        else {
            process(token_vector);
        }
    }
}

void lang::interpreter::read_from_file(const char *path) {
    // read from file path. Read line by line and call process_input on each line.
    auto scan = scanner();
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
        process_input(scan, &str);
    }
    file.close();


}

void lang::interpreter::error(std::string err) {
    std::cerr << "Error: " << err << std::endl;
}
