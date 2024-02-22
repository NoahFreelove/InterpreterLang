#ifndef GROUP_EVALUATOR_H
#define GROUP_EVALUATOR_H
#include "arithmetic_evaluator.h"
#include "truthy_evaluator.h"
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

        if(depth >= lang::interpreter::max_depth) {
            auto out = std::string("Max group evaluation depth reached: " + depth);
            out += " consider making your expression simpler, or an internal language error occured.";
            lang::interpreter::error(out);
            g->type = ERROR;
            return;
        }

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
                    stack_frame::eval_proc(g);
                    *g->tokens.erase(g->tokens.begin());
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

        // Check for CAST keyword, if its there cast the next token to the type two after the cast
        // Note even if the token after the cast is a group, we should evaluate it then cast if the type
        // of the group is not UNDERTERMINED or ERROR or NOTHING
        // We do the actual casting in a seperate method called cast_evaluator which requires the group
        // And the desired type as inputs. the desired type is an int
        // tokens is a vector of token elements, not tokens, you must use std::get to get the token
        for (int i = 0; i < g->tokens.size(); i++) {
            auto& element = g->tokens[i];
            if(!is_group(*element)) {
                auto t = std::get<std::shared_ptr<token>>(*element);
                if(t->get_name() == CAST && g->tokens.size() > i+2) {
                    std::shared_ptr<token> type_token = nullptr;
                    if(!is_group(*g->tokens[i+2])) {
                        type_token = std::get<std::shared_ptr<token>>(*g->tokens[i+2]);
                        if(!type_token->is_typeword()) {
                            g->type == ERROR;
                            lang::interpreter::error("Cannot use CAST without a typeword");
                            return;
                        }
                        std::any value = nullptr;
                        int current_type = 0;
                        int target_type = type_token->typeword_to_type();
                        if(is_group(*g->tokens[i+1])) {
                            auto to_cast_group = std::get<std::shared_ptr<token_group>>(*g->tokens[i+1]);
                            if(to_cast_group->type == UNDETERMINED) {
                                eval_group(to_cast_group, depth+1);
                            }
                            if(to_cast_group->type == NOTHING || to_cast_group->type == ERROR || to_cast_group->type == UNDETERMINED) {
                                lang::interpreter::error("Cannot cast nothing or an error to any value");
                                return;
                            }
                            value = to_cast_group->value;
                            current_type = to_cast_group->type;
                        }
                        else {
                            auto tok = std::get<std::shared_ptr<token>>(*g->tokens[i+1]);
                            current_type = tok->get_name();
                            value = tok->get_value();
                        }


                        std::pair<std::any, bool> result = cast_evaluator::eval(value, current_type, target_type);
                        if(result.second) {
                            if(is_group(*g->tokens[i+1])) {
                                auto evaled = std::get<std::shared_ptr<token_group>>(*g->tokens[i+1]);
                                evaled->type = target_type;
                                evaled->value = result.first;

                                if(target_type == BOOL_KEYW) {
                                    bool val = std::any_cast<bool>(result.first);
                                    evaled->type = (val? TRUE : FALSE);
                                    evaled->value = result.first;
                                }
                                else {
                                    evaled->type = target_type;
                                    evaled->value = result.first;
                                }
                            }
                            else {
                                auto tok = std::get<std::shared_ptr<token>>(*g->tokens[i+1]);
                                // make sure we account for bool edgecase
                                if(target_type == BOOL_KEYW) {
                                    bool val = std::any_cast<bool>(result.first);
                                    g->tokens[i+1] = convert(val? TRUE : FALSE, val? "true" : "false",0, val == true);
                                }
                                else {
                                    g->tokens[i+1] = convert(target_type, t->get_lexeme(), 0, result.first);
                                }
                            }
                            // erase tokens i+2 then tokens i
                            g->tokens.erase(g->tokens.begin() + i+2);
                            g->tokens.erase(g->tokens.begin() + i);
                        }else {
                            lang::interpreter::error("Failed cast from: " + id_to_name(current_type) + " to " + id_to_name(target_type));
                        }
                    }
                    else {
                        g->type == ERROR;
                        lang::interpreter::error("Cannot use CAST without a typeword");
                        return;
                    }

                    i+=2;
                }
            }
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
