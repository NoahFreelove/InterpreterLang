#include "group_evaluator.h"
#include "../tokenizer/token_grouper.h"
#include "truthy_evaluator.h"

data * group_evaluator::array_simplification(int i, const char *dat_name, const std::shared_ptr<token_group> &g,
    bool &allgood, data* preset) {
        data* base = nullptr;
        if(preset)
            base = preset;
        else
            base = resolve_variable(dat_name);
        if(!base) {
            lang::interpreter::error("Could not subscript array token. Array variable not found.");
            return nullptr;
        }

        std::vector<std::shared_ptr<token_element>> array_access;
        int j = i+1;
        bool found_open = false;
        bool found_close = false;
        if(j < g->tokens.size() && !is_group(*g->tokens[j])) {
            found_open = (std::get<std::shared_ptr<token>>(*g->tokens[j])->get_name() == LEFT_BRACKET);
        }
        j++;
        while(j < g->tokens.size()) {
            if(!is_group(*g->tokens[j])) {
                if(std::get<std::shared_ptr<token>>(*g->tokens[j])->get_name() == RIGHT_BRACKET) {
                    found_close = true;
                    break;
                }
            }

            array_access.push_back(g->tokens[j]);
            j++;
        }
        if(!found_open) {
            lang::interpreter::error("Array access must be followed by [");
            allgood = false;
            return nullptr;
        }
        if(!found_close) {
            lang::interpreter::error("Array access must be closed with ]");
            allgood = false;
            return nullptr;
        }
        // erase tokens i to j
        //std::cout << "before erase" << std::endl;
        g->tokens.erase(g->tokens.begin() + i, g->tokens.begin() + j+1);
        //std::cout << "after erase" << std::endl;

        auto tmp = std::make_shared<token_group>();
        tmp->tokens = array_access;
        std::vector<std::shared_ptr<token>> output;
        flatten(output, tmp);
        auto group = token_grouper::gen_group(output);
        eval_group(group);
        int index = 0;
        //std::cout << "TYPE: " << id_to_name(group->type) << std::endl;
        if(group->type != INT) {
            if(group->type == DOUBLE) {
                index = (int)(std::any_cast<double>(group->value));
            }
            else {
                lang::interpreter::error("Array subscript must contain an integer");
                return nullptr;
            }
        }
        else
            index = std::any_cast<int>(group->value);

        data* d = base->get_array_element(index);
        if(!d) {
            lang::interpreter::error("Array index out of bounds");
            return nullptr;
        }

        g->tokens.insert(g->tokens.begin() + i, convert(DATA, "data", 0, d));
        //std::cout << "after add" << std::endl;
        return d;
    }

void group_evaluator::eval_group(std::shared_ptr<token_group> g, int depth)  {

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
                    /*auto artificial_group = std::make_shared<token_group>();
                    artificial_group->tokens.push_back(g->tokens[0]);*/
                    stack_frame::eval_proc(g);

                    //std::cout << "AFTER PROC EVAL:" << tk->get_lexeme() << " : " << id_to_name(g->type) << std::endl;
                    //g->tokens[0] = std::make_shared<token_element>(artificial_group);

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