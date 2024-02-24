#ifndef GROUP_EVALUATOR_H
#define GROUP_EVALUATOR_H
#include "arithmetic_evaluator.h"
#include "../tokenizer/token_group.h"
#include "../tokenizer/token.h"
#include "../memory/stack_manager.h"
#include "cast_evaluator.h"
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

    static void flatten(std::vector<std::shared_ptr<token>>& output, const std::shared_ptr<token_group>& g) {
        for (auto& tok : g->tokens) {
            if(is_group(*tok)) {
                flatten(output, std::get<std::shared_ptr<token_group>>(*tok));
            }
            else {
                output.push_back(std::get<std::shared_ptr<token>>(*tok));
            }
        }
    }

    static data * array_simplification(int i, const char* dat_name, const std::shared_ptr<token_group>& g, bool& allgood);

    static bool recursive_replace(const std::shared_ptr<token_group>& g, bool simplify_array_dat = true) {
        bool allgood = true;
        for (int i = 0; i < g->tokens.size(); ++i) {
            if(!allgood)
                break;
            std::visit(overloaded{
                [g, &i, &allgood, &simplify_array_dat](const std::shared_ptr<token>& tk) {
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
                        data* d = nullptr;

                        if(i+1 < g->tokens.size()) {
                            if(!is_group(*g->tokens[i+1])) {
                                if (std::get<std::shared_ptr<token>>(*g->tokens[i+1])->get_name() == LEFT_BRACKET) {
                                    if(tk->get_name() == DATA) {
                                        d = std::any_cast<data*>(tk->get_value());
                                    }
                                    else {
                                        d = resolve_variable(tk->get_lexeme());
                                    }
                                }
                                else {
                                    d = resolve_variable(tk->get_lexeme());
                                }
                            }
                            else {
                                d = resolve_variable(tk->get_lexeme());
                            }
                        }
                        else {
                            d = resolve_variable(tk->get_lexeme());
                        }

                        if(!d) {
                            allgood = false;
                            lang::interpreter::error("Could not resolve variable " + std::string(tk->get_lexeme()));
                            return;
                        }

                        if(d->is_array()) {
                            d = array_simplification(i, tk->get_lexeme(), g, allgood);
                            if(!d) {
                                allgood = false;
                                lang::interpreter::error("Array access failed");
                                return;
                            }
                            if(d->is_array()) {
                                i--;
                                // the old array[] tokens are removed, and the new tokens are inserted so we want
                                // to continue from where we started.
                                return;
                            }
                            if(!simplify_array_dat) {
                                g->value = d;
                                g->type = DATA;
                                return;
                            }

                        }
                        //std::cout << tk->get_lexeme() << " <- var" << std::endl;
                        //lang::interpreter::stack->back()->dump_memory();
                        if (d) {
                            if(d->get_type_string() == "int") {
                                g->tokens[i] = convert(INT, "INT", 0, d->get_int());
                            }
                            else if(d->get_type_string() == "long") {
                                g->tokens[i] = convert(LONG, "LONG", 0, d->get_long());
                            }
                            else if(d->get_type_string() == "float") {
                                g->tokens[i] = convert(FLOAT, "FLOAT", 0, d->get_float());
                            }
                            else if(d->get_type_string() == "double") {
                                g->tokens[i] = convert(DOUBLE, "DOUBLE", 0, d->get_double());
                            }
                            else if (d->get_type_string() == "string") {
                                g->tokens[i] = convert(STRING, "STRING", 0, d->get_string());
                            }
                            else if(d->get_type_string() == "bool") {
                                bool val = d->get_bool();
                                if(!val) {
                                    g->tokens[i] = convert(FALSE, "FALSE", 0, false);
                                }
                                else {
                                    g->tokens[i] = convert(TRUE, "TRUE", 0, true);
                                }
                            }
                            else {
                                std::cout << "Unknown type: " << d->get_type_string() << std::endl;
                            }
                        }
                        else {
                            lang::interpreter::error("Variable not found: " +  std::string(tk->get_lexeme()));
                            allgood = false;
                        }
                    }
                },
                [&allgood, &simplify_array_dat](std::shared_ptr<token_group> grp) {
                    if(!recursive_replace(grp, simplify_array_dat)) {
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
    static void eval_group(std::shared_ptr<token_group> g, int depth = 0);
};



#endif //GROUP_EVALUATOR_H
