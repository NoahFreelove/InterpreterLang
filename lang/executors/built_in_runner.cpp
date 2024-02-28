#include "built_in_runner.h"
#include "../tokenizer/token.h"
#include "../interpreter.h"
#include "../memory/stack_manager.h"
#include "../memory/type_registry.h"
#include "../evaluator/group_evaluator.h"
#include <cstring>

void define(const std::vector<std::shared_ptr<token>> &tokens) {
    if (tokens.size() == 2) {
        lang::interpreter::defined->push_back(tokens[1]->get_lexeme());
        std::cout << tokens[1]->get_lexeme() << " defined" << std::endl;
    }
}

void undefine(const std::vector<std::shared_ptr<token>> &tokens) {
    if (tokens.size() == 2) {
        const char* name = tokens[1]->get_lexeme();
        for (int i = 0; i < lang::interpreter::defined->size(); i++) {
            if (strcmp(lang::interpreter::defined->at(i), name) == 0) {
                lang::interpreter::defined->erase(lang::interpreter::defined->begin() + i);
                std::cout << tokens[1]->get_lexeme() << " undefined" << std::endl;
                return;
            }
        }
        std::cout << tokens[1]->get_lexeme() << " not defined" << std::endl;
    }
}

void is_defined(proc_type_vec& args) {
    data* var = resolve_variable(args[0].second->get_lexeme());
    if(!var) {
        lang::interpreter::error("Native method is_defined input variable is non existent");
        return;
    }

    if(var->get_type_int() != STRING) {
        lang::interpreter::error("Native method is_defined argument type is not string");
        return;
    }

    data* return_var = resolve_variable("return");
    if(!return_var) {
        lang::interpreter::error("Native method is_defined could not access return variable");
        return;
    }

    if(return_var->get_type_int() != BOOL_KEYW) {
        lang::interpreter::error("Native method is_defined return type is not string");
    }
    return_var->set_value_bool(lang::interpreter::is_defined(var->to_string().c_str()));
    return;
}

void delete_var(proc_type_vec& args) {
    data* var = resolve_variable(args[0].second->get_lexeme());
    if(!var) {
        lang::interpreter::error("Native method delete input variable is non existent");
        return;
    }



    data* return_var = resolve_variable("return");
    if(!return_var) {
        lang::interpreter::error("Native method delete could not access return variable");
        return;
    }

    if(return_var->get_type_int() != NOTHING) {
        lang::interpreter::error("Native method delete return type is not void");
    }

    delete_variable(args[0].second->get_lexeme(), false);
}

void print(const std::vector<std::shared_ptr<token>> &tokens, int offset) {
    if(tokens.size() < 2) {
        lang::interpreter::error("Not enough tokens for print statement");
        return;
    }

    /*std::cout << "ALL TOKENS: " << std::endl;
    for (auto& token : tokens) {
        std::cout << *token << std::endl;
    }*/

    auto group = lang::interpreter::evaluate_tokens(tokens, offset);
    if(group->type == UNDETERMINED || group->type == ERROR) {
        std::cout << std::endl;
    }

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
        std::cout << std::any_cast<std::string>(group->value) << std::endl;
    }
    else if(group->type == TRUE) {
        if(std::any_cast<bool>(group->value)) {
            std::cout << "true" << std::endl;
        }
    }
    else if(group->type == FALSE) {
        if(!std::any_cast<bool>(group->value)) {
            std::cout << "false" << std::endl;
        }
    }
    else {
        std::cout << id_to_name(tokens[1]->get_name()) << std::endl;
    }
}

void process_import(std::vector<std::shared_ptr<token>> tokens) {

    if (tokens.size() != 2) {
        lang::interpreter::error("Invalid usage of import: import <\"filepath\">");
        return;
    }
    if(tokens[1]->get_name() != STRING) {
        lang::interpreter::error("Invalid usage of import. Filepath must be a STRING: import <\"filepath\">");
        return;
    }
    lang::interpreter::read_from_file(tokens[1]->get_lexeme());
}

void dump()  {
    for(stack_frame* frame : *lang::interpreter::stack) {
        frame->dump_memory();
        std::cout << std::endl;
    }
}

void assert(std::vector<std::shared_ptr<token>> tokens) {
    tokens.erase(tokens.begin());
    auto group = token_grouper::gen_group(tokens);
    std::string pre_group;
    group->output_group(pre_group, 0);
    group_evaluator::recursive_replace(group);
    std::string printout;
    group->output_group(printout, 0);

    group_evaluator::eval_group(group);
    if(group->type == TRUE || group->type == FALSE) {
        if(group->type == FALSE) {
            lang::interpreter::error("Assertion error - False: " + printout + "\nBefore grouping: " + pre_group);
        }
    }
    else {
        lang::interpreter::error("Assertion Error: Assertion not truthy");
    }
}


void execute_typeof(proc_type_vec& args) {
    data* var = resolve_variable(args[0].second->get_lexeme());
    if(!var) {
        lang::interpreter::error("Native method typeof input variable is non existent");
        return;
    }

    data* return_var = resolve_variable("return");
    if(!return_var) {
        lang::interpreter::error("Native method typeof could not access return variable");
        return;
    }

    if(return_var->get_type_int() != STRING) {
        lang::interpreter::error("Native method typeof return type is not string");
    }

    return_var->set_value_string(var->get_type_string());

}


void execute_internal_method(const std::string& proc_name, proc_type_vec args) {
    /*std::cout << "Native proc name: " << proc_name << std::endl;
    std::cout << "Native proc types: "<< std::endl;
    for (auto& vec : args) {
        std::cout << vec.second->get_lexeme() << std::endl;
    }*/

    if(proc_name == "dump" && args.empty()) {
        dump();
    }

    if(proc_name == "typeof" && args.size() == 1) {
        execute_typeof(args);
    }
    if(proc_name == "is_defined" && args.size() == 1) {
        is_defined(args);
    }
    if(proc_name == "delete" && args.size() == 1) {
        delete_var(args);
    }
}

void run_builtins(const std::vector<std::shared_ptr<token>> &tokens) {
    switch (tokens[0]->get_name()) {
        case PRINT: {
            print(tokens);
            break;
        }
        case DEFINE: {
            define(tokens);
            break;
        }
        case UNDEFINE: {
            undefine(tokens);
            break;
        }
        case IMPORT: {
            process_import(tokens);
            break;
        }
        case ID: {
            print(tokens,0);
            break;
        }
        case ASSERT: {
            assert(tokens);
            break;
        }
        case EXIT: {
            lang::interpreter::exit();
            break;
        }
        default: {
            lang::interpreter::error("unknown built-in function: " + id_to_name(tokens[0]->get_name()));
        }
    }
}