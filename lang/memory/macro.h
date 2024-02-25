#ifndef MACRO_H
#define MACRO_H
#include "../interpreter.h"
#include "../evaluator/group_evaluator.h"
#include <cstring>
class macro {
    std::vector<std::string>* vars;
    std::vector<std::shared_ptr<token>>* expression;

public:
    explicit macro(const std::vector<const char*>& vars, const std::vector<std::shared_ptr<token>> &expression) {
        this->vars = new std::vector<std::string>();
        this->vars->reserve(vars.size());
        for (auto& var : vars) {
            this->vars->push_back(std::string(var));
        }
        this->expression = new std::vector<std::shared_ptr<token>>();
        this->expression->reserve(expression.size());
        for(auto& ptr: expression) {
            this->expression->push_back(std::make_shared<token>(ptr.get()));
        }
    }
    ~macro() {
        delete vars;
        delete expression;
    }
    std::shared_ptr<token_group> convert_input_to_macro(const std::vector<std::shared_ptr<token_group>>& args) {
        if(args.size() != vars->size()) {
            lang::interpreter::error("Macro arg size doesn't match");
            return std::make_shared<token_group>(ERROR, nullptr);
        }
        auto clone = lang::interpreter::clone_tokens(*expression);

        int i = 0;
        // Replace each macro variable (eg, x, y) that is in the expression (eg. x+y) with the
        // tokens that are in the args argument. They are replaced in the order they appear in the this.vars vector
        // ex. input: args = (5+6),(2) vars = "x","y", expression = "y*(x)", reuslt: "2*(5+6)"
        for (const auto& arg: args) {
            std::vector<std::shared_ptr<token>> replacement;
            // groups are easier for the grouper to work with, but not for us,
            // so we flatten them into a vector.
            group_evaluator::flatten(replacement, arg);
            for (int j = 0; j < clone.size(); j++) {
                auto& tok = clone[j];
                std::string lexeme = std::string(tok->get_lexeme());
                if(lexeme == (*vars)[i]) {
                    clone.erase(clone.begin() + j);
                    for(int k = 0; k < replacement.size(); k++) {
                        if(k+j >= clone.size()) {
                            clone.push_back(replacement[k]);
                        }
                        else {
                            clone.insert(clone.begin()+j+k,replacement[k]);
                        }
                    }
                }
            }
            i++;
        }

        auto output = token_grouper::gen_group(clone);
        //output->print_group();
        group_evaluator::eval_group(output);
        return output;
    }

    static macro* gen_macro(std::vector<std::shared_ptr<token>> tokens) {
        if(!tokens[1]->is_identifier()) {
            lang::interpreter::error("Expected macro name in macro declaration");
            return nullptr;
        }

        tokens.erase(tokens.begin(), tokens.begin()+2);

        std::vector<const char*> names;
        if(tokens[0]->get_name() != COLON) {
            while (!tokens.empty()) {
                if(tokens[0]->get_name() == COLON) {
                    break;
                }
                if(tokens[0]->get_name() == COMMA)
                    tokens.erase(tokens.begin());
                else if(tokens[0]->is_identifier()) {
                    names.push_back(tokens[0]->get_lexeme());
                    //std::cout << "found arg: " << tokens[0]->get_lexeme() <<std::endl;
                    tokens.erase(tokens.begin());
                }
                else {
                    lang::interpreter::error("Illegal token in macro declaration: " + id_to_name(tokens[0]->get_name()));
                    return nullptr;
                }
            }
            if(tokens.empty()) {
                lang::interpreter::error("Expected : in macro declaration");
                return nullptr;
            }
        }

        tokens.erase(tokens.begin());
        std::vector<std::shared_ptr<token>> expression = std::vector(tokens);

        return new macro(names, expression);
    }
};
#endif //MACRO_H
