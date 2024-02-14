#ifndef GROUP_EVALUATOR_H
#define GROUP_EVALUATOR_H
#include "arithmetic_evaluator.h"
#include "truthy_evaluator.h"
#include "../tokenizer/token_group.h"
#include "../tokenizer/token.h"
#include "../memory/stack_manager.h"
class group_evaluator {
public:
    using token_element = std::variant<std::shared_ptr<token>, std::shared_ptr<token_group>>;
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    static std::shared_ptr<token_element> convert(int name, const char* lexeme, int line, std::any value) {
        return std::make_shared<token_element>(std::make_shared<token>(name, lexeme, 0, value));
    }

    static const char* string_to_const_char(const std::string& input) {
        // copy value completely
        char* name = (char*)malloc(sizeof(char)*input.size());
        return strcpy(name, input.c_str());
    }

    static bool recursive_replace(const std::shared_ptr<token_group>& g) {
        bool allgood = true;
        for (int i = 0; i < g->tokens.size(); ++i) {
            if(!allgood)
                break;
            std::visit(overloaded{
                [g, &i, &allgood](const std::shared_ptr<token>& tk) {
                    // Usage of ID:  id "some_string", converts to IDENTIFIER: some_string
                    if(tk->get_name() == ID && i+1 < g->tokens.size()) {
                        std::string name;
                        std::visit(overloaded{
                            [g, i, &name](const std::shared_ptr<token>& tk) {
                                if(tk->get_name() == STRING) {
                                    name = std::any_cast<std::string>(tk->get_value());
                                }
                                else if(tk->get_name() == IDENTIFIER) {
                                    data* d =  resolve_variable(tk->get_lexeme());
                                    if(d) {
                                        if(d->get_type() == "string") {
                                            name = d->get_string();
                                        }
                                        else {
                                            lang::interpreter::error("invalid use of ID, a string identifier must follow");
                                        }
                                    }
                                }
                                else {
                                    lang::interpreter::error("invalid use of ID, a string must follow");
                                }
                            },
                                [&name](const std::shared_ptr<token_group>& grp) {
                                    lang::interpreter::error("invalid use of ID, a string must follow, not a group");
                            }
                        }, *g->tokens[i+1]);

                        if(!name.empty()) {
                            auto new_token = convert(IDENTIFIER, string_to_const_char(name), 0, name);
                            g->tokens[i+1] = new_token;
                            g->tokens.erase(g->tokens.begin() + i);
                            i--;
                        }

                    }
                    else if(tk->get_name() == IDENTIFIER) {
                        // Replace token in group with value obtained from memory
                        data* d = resolve_variable(tk->get_lexeme());
                        if (d) {
                            if(d->get_type() == "int") {
                                g->tokens[i] = convert(INT, "INT", 0, d->get_int());
                            }
                            if(d->get_type() == "long") {
                                g->tokens[i] = convert(LONG, "LONG", 0, d->get_long());
                            }
                            else if(d->get_type() == "float") {
                                g->tokens[i] = convert(FLOAT, "FLOAT", 0, d->get_float());
                            }
                            else if(d->get_type() == "double") {
                                g->tokens[i] = convert(DOUBLE, "DOUBLE", 0, d->get_double());
                            }
                            else if (d->get_type() == "string") {
                                g->tokens[i] = convert(STRING, "STRING", 0, d->get_string());
                            }
                            else if(d->get_type() == "bool") {
                                bool val = d->get_bool();
                                if(!val) {
                                    g->tokens[i] = convert(FALSE, "FALSE", 0, false);
                                }
                                else {
                                    g->tokens[i] = convert(TRUE, "TRUE", 0, true);
                                }
                            }
                        }
                        else {
                            lang::interpreter::error("Variable not found");
                            allgood = false;
                        }
                    }
                },
                [&allgood](std::shared_ptr<token_group> grp) {
                    if(!recursive_replace(grp)) {
                        allgood = false;
                    }
                }
            }, *g->tokens[i]);
        }
        return allgood;
    }

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

    static void set_group_val(std::shared_ptr<token_group>& g) {
        std::shared_ptr<token> t = std::get<std::shared_ptr<token>>(*g->tokens[0]);
        if(t->get_name() == TRUE) {
            g->type = TRUE;
            g->value = true;
        }
        else if(t->get_name() == FALSE) {
            g->type = FALSE;
            g->value = false;
        }
        else {
            g->type = t->get_name();
            g->value = t->get_value();
        }
    }
    static void eval_group(std::shared_ptr<token_group> g, int depth = 0) {
        bool result = recursive_replace(g);

        if(g->tokens.empty()) {
            g->type = NOTHING;
            g->value = nullptr;
            return;
        }
        // check for proc, if it is it should always be in its own group
        if(g->tokens.size() == 1) {
            if(!is_group(*g->tokens[0])) {
                auto tk = std::get<std::shared_ptr<token>>(*g->tokens[0]);
                if(tk->get_name() == PROC) {
                    lang::interpreter::top_stack()->eval_proc(g);
                    return;
                }

            }
        }

        //g->print_group();
        if(!result) {
            g->type = ERROR;
            g->value = nullptr;
            return;
        }
        bool has_arithmetic = false;
        bool is_pure_arithmetic = true;
        bool has_err = false;
        //std::cout << "Len: " << g->tokens.size() << std::endl;
        for (const std::shared_ptr<token_element>& element : g->tokens) {
            std::visit(overloaded{
                [g, &has_arithmetic, &is_pure_arithmetic](const std::shared_ptr<token>& tk) {
                    if(tk->is_arithmetic()) {
                        has_arithmetic = true;
                    }
                    else if (tk->get_name() != IDENTIFIER && !tk->is_numeric()) {
                        is_pure_arithmetic = false;
                    }
                },
                [g, depth, &has_err](std::shared_ptr<token_group> grp) {
                    if(grp->type == UNDETERMINED)
                        eval_group(grp,depth+1);
                    if(grp->type == ERROR) {
                        has_err = true;
                    }
                }
            }, *element);
        }
        if(has_err) {
            g->type = ERROR;
            g->value = nullptr;
            return;
        }

        bool has_literal = false;
        for (int i = 0; i< g->tokens.size(); i++) {
            auto& element = g->tokens[i];
            std::visit(overloaded{
                [g, &has_literal](std::shared_ptr<token> tk) {
                    if(tk->is_literal()) {
                        has_literal = true;
                    }
                },
                [&g, depth, &has_err, i](std::shared_ptr<token_group> grp) {
                    if(grp->type == UNDETERMINED)
                        eval_group(grp,depth+1);
                    if(grp->type == NOTHING) {
                        // remove the group
                        g->tokens.erase(g->tokens.begin() + i);
                    }

                    if(grp->type == ERROR) {
                        has_err = true;
                    }
                }
            }, *element);
        }

        if(has_err) {
            g->type = ERROR;
            g->value = nullptr;
            return;
        }

        //g->print_group();

        // We need to evaluate methods before arithmetic or logic
        // if the methods require some arithmetic or logic, run those evaulations in the method call parenthesis
        // method(5+5 == 10) -> 5+5 -> 10 == 10 -> method(true)

        // ReSharper disable once CppDFAConstantConditions
        if(has_arithmetic || has_literal) {
            arithmetic_evaluator::recursive_evaluation(g);
        }

        if(!is_pure_arithmetic) {
            //std::cout << "not purely arithmetic" << std::endl;
            g->value = nullptr;
            g->type = UNDETERMINED;
        }
        if (g->tokens.size() == 1) {
            if(is_group(*g->tokens[0])) {
                auto group = std::get<std::shared_ptr<token_group>>(*g->tokens[0]);
                g->value = group->value;
                g->type = group->type;
            }
            else {
                set_group_val(g);
            }
            return;

        }

        // now that its evaluated, we check truthy values
        truthy_evaluator::truth_eval(g);

        // if there is only one token, set the group type and value to that token
        if (g->tokens.size() == 1) {
            if(is_group(*g->tokens[0])) {
                auto group = std::get<std::shared_ptr<token_group>>(*g->tokens[0]);
                g->value = group->value;
                g->type = group->type;
            }
            else {
                set_group_val(g);
            }
        }

    }
};



#endif //GROUP_EVALUATOR_H
