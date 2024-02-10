#ifndef TRUTHY_EVALUATOR_H
#define TRUTHY_EVALUATOR_H
#include "../tokenizer/token_group.h"
#include "../tokenizer/token.h"
class truthy_evaluator {
    using token_element = std::variant<std::shared_ptr<token>, std::shared_ptr<token_group>>;
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
    inline static token* invalid_ant = new token(INT, "INT", 0, 0);
    static bool is_group(const token_element& t) {
        return std::visit(overloaded{
                [](const std::shared_ptr<token>& tk) {
                    return false;
                },
                [](const std::shared_ptr<token_group>& grp) {
                    return true;
                }
            }, t);
    }

    static std::shared_ptr<token_element> convert(int name, const char* lexeme, int line, std::any value) {
        return std::make_shared<token_element>(std::make_shared<token>(name, lexeme, 0, value));
    }

    static void recursive_evaluation(std::shared_ptr<token_group> g) {
        std::vector<std::shared_ptr<token_element>>& tokens = g->tokens;
        bool groups_found = true;
        while (groups_found) {
            groups_found = false;
            for (int i = 0; i < tokens.size(); ++i) {
                if(is_group(*tokens[i])) {
                    groups_found = true;
                    auto tg = std::get<std::shared_ptr<token_group>>(*tokens[i]);
                    if(tg->type == UNDETERMINED) {
                        recursive_evaluation(tg);
                    }
                    if(tg->type == TRUE) {
                        g->tokens[i] = convert(TRUE, "TRUE", 0, true);
                    }
                    if(tg->type == FALSE) {
                        g->tokens[i] = convert(FALSE, "FALSE", 0, false);
                    }
                }
            }
        }
        // if there is a single true or false token, just set this groups' value to that
        if (tokens.size() == 1) {
            auto t = std::get<std::shared_ptr<token>>(*tokens[0]);
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
            for (auto & i : tokens) {
                auto t = std::get<std::shared_ptr<token>>(*i);
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
            std::shared_ptr<token> ant;
            std::shared_ptr<token> cons;
            std::shared_ptr<token> op;

            // before we eval, we must check for if ! is attached to a lone token
            // if so, we must evaluate it first

            for (int i = 0; i < tokens.size(); ++i) {
                std::shared_ptr<token> t;
                if(is_group(*tokens[i])) {
                    std::cout << "GROUP" << std::endl;
                    return;
                }
                else {
                    t = std::get<std::shared_ptr<token>>(*tokens[i]);
                }
                if(t->get_name() == BANG) {
                    if(i+1 < tokens.size()) {
                        t = std::get<std::shared_ptr<token>>(*tokens[i+1]);
                        if(t->get_name() == TRUE) {
                            tokens[i] = convert(FALSE, "FALSE", 0, false);
                            tokens.erase(tokens.begin() + i + 1);
                        }
                        else if (t->get_name() == FALSE) {
                            tokens[i] = convert(TRUE, "TRUE", 0, true);
                            tokens.erase(tokens.begin() + i + 1);
                        }
                        else {
                            lang::interpreter::error("cannot apply ! to non-logical value");
                        }
                    }
                }
            }
            //g->print_group();

            for (int i = 0; i < tokens.size(); ++i) {
                std::shared_ptr<token> t = nullptr;
                if(is_group(*tokens[i])) {
                    std::cout << "GROUP" << std::endl;
                    return;
                }
                else {
                    t = std::get<std::shared_ptr<token>>(*tokens[i]);
                }
                if(t->is_logical()) {
                    // do and and or first because they have lower precedence over
                    if(t->get_name() == AND) {
                        ant_index = i - 1;
                        ant = std::get<std::shared_ptr<token>>(*tokens[ant_index]);
                        cons = std::get<std::shared_ptr<token>>(*tokens[i + 1]);
                        op = t;
                        has_ops = true;
                        break;
                    }
                    else if(t->get_name() == OR) {
                        ant_index = i - 1;
                        ant = std::get<std::shared_ptr<token>>(*tokens[ant_index]);
                        cons = std::get<std::shared_ptr<token>>(*tokens[i + 1]);
                        op = t;
                        has_ops = true;
                        break;
                    }
                }
            }
        }
    }
public:
    static void truth_eval(std::shared_ptr<token_group> g) {
        //recursive_evaluation(g);
    }
};
#endif //TRUTHY_EVALUATOR_H
