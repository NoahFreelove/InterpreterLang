#ifndef TRUTHY_EVALUATOR_H
#define TRUTHY_EVALUATOR_H
#include "../tokenizer/token_group.h"
#include "../tokenizer/token.h"
class truthy_evaluator {
    using token_element = std::variant<token*, token_group*>;
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
    inline static token* invalid_ant = new token(INT, "INT", 0, 0);
    static bool is_group(const token_element& t) {
        return std::visit(overloaded{
                [](token* tk) {
                    return false;
                },
                [](token_group* grp) {
                    return true;
                }
            }, t);
    }

    static void recursive_evaluation(token_group* g) {
        std::vector<token_element>& tokens = g->tokens;
        bool groups_found = true;
        while (groups_found) {
            groups_found = false;
            for (int i = 0; i < tokens.size(); ++i) {
                if(is_group(tokens[i])) {
                    groups_found = true;
                    token_group* tg = std::get<token_group*>(tokens[i]);
                    if(tg->type == UNDETERMINED) {
                        recursive_evaluation(tg);
                    }
                    if(tg->type == TRUE) {
                        g->tokens[i] = new token(TRUE, "TRUE", 0, true);
                    }
                    if(tg->type == FALSE) {
                        g->tokens[i] = new token(FALSE, "FALSE", 0, false);
                    }
                }
            }
        }
        // if there is a single true or false token, just set this groups' value to that
        if (tokens.size() == 1) {
            token* t = std::get<token*>(tokens[0]);
            if(t->get_name() == TRUE) {
                g->type = TRUE;
            }
            else if(t->get_name() == FALSE) {
                g->type = FALSE;
            }
        }
        else {
            // if there are no logical operators, then we can just set the type to UNDETERMINED
            bool has_ops = false;
            for (int i = 0; i < tokens.size(); ++i) {
                token* t = std::get<token*>(tokens[i]);
                if(t->is_logical()) {
                    has_ops = true;
                    break;
                }
            }
            if(!has_ops) {
                g->type = UNDETERMINED;
            }
        }

        bool has_ops = true;

        while (has_ops) {
            has_ops = false;
            int ant_index = -1;
            token* ant = nullptr;
            token* cons = nullptr;
            token* op = nullptr;

            // before we eval, we must check for if ! is attached to a lone token
            // if so, we must evaluate it first

            for (int i = 0; i < tokens.size(); ++i) {
                token* t = nullptr;
                if(is_group(tokens[i])) {
                    std::cout << "GROUP" << std::endl;
                    return;
                }
                else {
                    t = std::get<token*>(tokens[i]);
                }
                if(t->get_name() == BANG) {
                    if(i+1 < tokens.size()) {
                        t = std::get<token*>(tokens[i+1]);
                        if(t->get_name() == TRUE) {
                            tokens[i] = new token(FALSE, "FALSE", 0, false);
                            tokens.erase(tokens.begin() + i + 1);
                            delete t;
                        }
                        else if (t->get_name() == FALSE) {
                            tokens[i] = new token(TRUE, "TRUE", 0, true);
                            tokens.erase(tokens.begin() + i + 1);
                            delete t;
                        }
                        else {
                            lang::interpreter::error("cannot apply ! to non-logical value");
                        }
                    }
                }
            }
            //g->print_group();

            for (int i = 0; i < tokens.size(); ++i) {
                token* t = nullptr;
                if(is_group(tokens[i])) {
                    std::cout << "GROUP" << std::endl;
                    return;
                }
                else {
                    t = std::get<token*>(tokens[i]);
                }
                if(t->is_logical()) {
                    // do and and or first because they have lower precedence over
                    if(t->get_name() == AND) {
                        ant_index = i - 1;
                        ant = std::get<token*>(tokens[ant_index]);
                        cons = std::get<token*>(tokens[i + 1]);
                        op = t;
                        has_ops = true;
                        break;
                    }
                    else if(t->get_name() == OR) {
                        ant_index = i - 1;
                        ant = std::get<token*>(tokens[ant_index]);
                        cons = std::get<token*>(tokens[i + 1]);
                        op = t;
                        has_ops = true;
                        break;
                    }
                }
            }
        }
    }
public:
    static void truth_eval(token_group* g) {
        recursive_evaluation(g);
    }
};
#endif //TRUTHY_EVALUATOR_H
