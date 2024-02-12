#ifndef BUILT_IN_RUNNER_H
#define BUILT_IN_RUNNER_H
#include "../tokenizer/token.h"
#include "../interpreter.h"

void define(const std::vector<std::shared_ptr<token>> &tokens) {
    if (tokens.size() == 2) {
        lang::interpreter::defined->push_back(lang::interpreter::const_char_convert(tokens[1]->get_lexeme()));
        std::cout << tokens[1]->get_lexeme() << " defined" << std::endl;
    }
}

void undefine(const std::vector<std::shared_ptr<token>> &tokens) {
    if (tokens.size() == 2) {
        char* name = lang::interpreter::const_char_convert(tokens[1]->get_lexeme());
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

void is_defined(const std::vector<std::shared_ptr<token>> &tokens) {
    if (tokens.size() == 2) {
        char* name = lang::interpreter::const_char_convert(tokens[1]->get_lexeme());
        for (int i = 0; i < lang::interpreter::defined->size(); i++) {
            if (strcmp(lang::interpreter::defined->at(i), name) == 0) {
                std::cout << tokens[1]->get_lexeme() << " is defined" << std::endl;
                return;
            }
        }
        std::cout << tokens[1]->get_lexeme() << " is not defined" << std::endl;
    }
}

void delete_var(const std::vector<std::shared_ptr<token>> &tokens) {
    if(tokens.size() == 2) {
        if(tokens[0]->get_name() == DELETE && tokens[1]->get_name() == IDENTIFIER) {
            lang::interpreter::stack->top()->delete_var(lang::interpreter::const_char_convert(tokens[1]->get_lexeme()));
        }
    }
}

static void print(const std::vector<std::shared_ptr<token>>& tokens, int offset = 1) {
    if(tokens.size() < 2) {
        lang::interpreter::error("Not enough tokens for print statement");
        return;
    }
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

static void process_import(std::vector<std::shared_ptr<token>> tokens) {

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

static void run_builtins(const std::vector<std::shared_ptr<token>>& tokens) {
    if (tokens[0]->get_name() == PRINT) {
        print(tokens);
        return;
    }
    if (tokens[0]->get_name() == DUMP) {
        lang::interpreter::stack->top()->dump_memory();
        return;
    }
    if (tokens[0]->get_name() == DEFINE) {
        define(tokens);
        return;
    }
    if (tokens[0]->get_name() == UNDEFINE) {
        undefine(tokens);
        return;
    }
    if (tokens[0]->get_name() == ISDEFINED) {
        is_defined(tokens);
        return;
    }
    if (tokens[0]->get_name() == DELETE) {
        delete_var(tokens);
        return;
    }
    if(tokens[0]->get_name() == IMPORT) {
        process_import(tokens);
        return;
    }
    if(tokens[0]->get_name() == ID) {
        print(tokens,0);
        return;
    }
}
#endif //BUILT_IN_RUNNER_H
