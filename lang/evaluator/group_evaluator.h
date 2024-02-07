#ifndef GROUP_EVALUATOR_H
#define GROUP_EVALUATOR_H
#include "arithmetic_evaluator.h"
#include "../tokenizer/token_group.h"
#include "../tokenizer/token.h"
class group_evaluator {
public:
    using token_element = std::variant<token*, token_group*>;
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
    static void recursive_replace(token_group* g) {
        int i = 0;
        for (const token_element& element : g->tokens) {
            std::visit(overloaded{
                [g, i](token* tk) {
                    if(tk->get_name() == IDENTIFIER) {
                        // Replace token in group with value obtained from memory
                        data* d = lang::interpreter::stack->top()->get_data(lang::interpreter::const_char_convert(tk->get_lexeme()));
                        if (d) {
                            if(d->get_type() == "int") {
                                g->tokens[i] = new token(108, "INT", 0, d->get_int());
                            }
                            else if(d->get_type() == "float") {
                                g->tokens[i] = new token(104, "FLOAT", 0, d->get_float());
                            }
                            else if(d->get_type() == "double") {
                                g->tokens[i] = new token(105, "DOUBLE", 0, d->get_double());
                            }
                            else if (d->get_type() == "string") {
                                g->tokens[i] = new token(102, "STRING", 0, d->get_string());
                            }
                            else if(d->get_type() == "bool") {
                                bool val = d->get_bool();
                                if(val) {
                                    g->tokens[i] = new token(110, "FALSE", 0, false);
                                }
                                else {
                                    g->tokens[i] = new token(109, "TRUE", 0, true);
                                }
                            }
                        }
                        else {
                            lang::interpreter::error("Variable not found");
                        }
                    }
                },
                [](token_group* grp) {
                    recursive_replace(grp);
                }
            }, element);
            i+=1;
        }
    }
    static void eval_group(token_group* g) {
        recursive_replace(g);
        arithmetic_evaluator::recursive_evaluation(g);
    }
};



#endif //GROUP_EVALUATOR_H
