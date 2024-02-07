#ifndef ARITHMETIC_EVALUATOR_H
#define ARITHMETIC_EVALUATOR_H
#include <complex>

#include "../tokenizer/token_group.h"
#include "../tokenizer/token.h"
class arithmetic_evaluator {
public:
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
    static bool has_next(token_group* g, int i, int inc) {
        return (i+inc) < g->tokens.size();
    }

    static bool check_errs(std::vector<token_element> tokens) {
        token* last_token = nullptr;
        for (int i = 0; i < tokens.size(); ++i) {
            token* curr = std::get<token*>(tokens[i]);
            if(i == 0 && curr->is_arithmetic() && curr->is_add_sub()) {
                lang::interpreter::error("operator with no antecedent");
                return false;
            }
            if(i == tokens.size() - 1 && curr->is_arithmetic()) {
                lang::interpreter::error("operator with no consequent");
                return false;
            }

            if(last_token != nullptr) {
                if(last_token->is_arithmetic() && curr->is_arithmetic()) {
                    if(!last_token->is_add_sub() || curr->get_name() != MINUS) {
                        lang::interpreter::error("two operators side by side");
                        return false;
                    }
                }
                if(last_token->is_numeric() && curr->is_numeric()) {
                    lang::interpreter::error("two is_numeric side by side without an operator");
                    return false;
                }
            }
            else {
                last_token = curr;
            }
        }
        return true;
    }

    static void recursive_evaluation(token_group* g) {
        // Following BEDMAS rules, recursively evaluate expressions which are in the form of a group
        // At this point everything should be a primitive, so if a group is found, it should be evaluated
        // If a string is found to be used in an operation (i.e. not alone), throw an error
        // If a bool is found to be used in an operation (i.e. not alone), throw an error
        // only evaluate arithmetic operators such as +-*/. Operators like ==, !=, <, >, <=, >= should be evaluated in a different function
        // The type of the left side takes precedence. double > float > int. If the left side is a double, the right side will be converted to a double
        // However doubles or floats cannot be converted to int unless IMPLICIT_CONVERSION is defined, then we floor it
        // Each group has a type, which is the type of the result of the evaluation
        // Each group also has a value, which is the result of the evaluation
        // if the type is undetermined, it is either a group or has not been evaluated yet
        //std::cout << "recursive eval" << std::endl;
        //std::cout << g->tokens.size() << std::endl;

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
                    if(tg->type == INT) {
                        g->tokens[i] = new token(INT, "INT", 0, tg->value);
                    }
                    if(tg->type == FLOAT) {
                        g->tokens[i] = new token(FLOAT, "FLOAT", 0, tg->value);
                    }
                    if(tg->type == DOUBLE) {
                        g->tokens[i] = new token(DOUBLE, "DOUBLE", 0, tg->value);
                    }
                    if(tg->type == LONG) {
                        g->tokens[i] = new token(LONG, "LONG", 0, tg->value);
                    }
                    if(tg->type == ULONG64) {
                        g->tokens[i] = new token(ULONG64, "ULONG64", 0, tg->value);
                    }
                }
            }
        }

        if(g->tokens.size() == 1) {
            if (!is_group(g->tokens[0])) {
                g->type = INT;
                g->value = std::get<token*>(g->tokens[0])->get_value();
                return;
            }
        }

        // all parenthesis are gone, all groups evaluated, just eval.
        bool has_ops = true;

        if(!check_errs(tokens))
            return;

        while (has_ops) {
            has_ops = false;
            // find antecedent and consequent of any operators
            // Do one pass to find * or /. If they are found find the antecedent and concequent
            // If these aren't found, find the first of + or -
            // Replace the op, ant, and cons with a new token of the arithmetic value
            int ant_index = -1;
            token* ant = nullptr;
            token* cons = nullptr;
            token* op = nullptr;
            for (int i = 0; i < tokens.size(); ++i) {
                token* t = nullptr;
                // if its an instance of group, cout it
                if(is_group(tokens[i])) {
                    std::cout << "GROUP" << std::endl;
                    return;
                }
                else {
                    t = std::get<token*>(tokens[i]);
                }

                if(t->is_arithmetic()) {
                    if(t->get_name() == EXPONENT) {
                        if(has_next(g, i, 1)) {
                            ant_index = i-1;
                            ant = std::get<token*>(tokens[i-1]);
                            cons = std::get<token*>(tokens[i+1]);
                            op = std::get<token*>(tokens[i]);
                            has_ops = true;
                            break;
                        }
                    }
                    if(t->get_name() == STAR || t->get_name() == SLASH) {
                        //std::cout << "star or slash" << std::endl;
                        if(has_next(g, i, 1)) {
                            ant_index = i-1;
                            ant = std::get<token*>(tokens[i-1]);
                            cons = std::get<token*>(tokens[i+1]);
                            op = std::get<token*>(tokens[i]);
                            has_ops = true;
                            break;
                        }
                    }
                    if(t->get_name() == PLUS || t->get_name() == MINUS) {
                        //std::cout << "add or sub" << std::endl;

                        if(has_next(g, i, 1)) {
                            ant_index = i-1;
                            if(ant_index < 0) {
                                ant = nullptr;
                            }
                            else
                                ant = std::get<token*>(tokens[i-1]);

                            op = std::get<token*>(tokens[i]);
                            cons = std::get<token*>(tokens[i+1]);
                            // if cons is a minus, take the value after it, and negate it, then replace the cons with the new value
                            // after the minus and delete the minus so its just an addition
                            if(cons->get_name() == MINUS && op->is_arithmetic()) {
                                //g->print_group();
                                //std::cout << "replacing" << std::endl;
                                if(!has_next(g,i,2))
                                    continue;
                                token* cons2 = std::get<token*>(tokens[i+2]);
                                if(cons2->get_name() == INT) {
                                    cons2->set_value(-std::any_cast<int>(cons2->get_value()));
                                }
                                else if(cons2->get_name() == FLOAT) {
                                    cons2->set_value(-std::any_cast<float>(cons2->get_value()));
                                }
                                else if(cons2->get_name() == DOUBLE) {
                                    cons2->set_value(-std::any_cast<double>(cons2->get_value()));
                                }
                                else if(cons2->get_name() == LONG) {
                                    cons2->set_value(-std::any_cast<long>(cons2->get_value()));
                                }
                                else if(cons2->get_name() == ULONG64) {
                                    cons2->set_value(-std::any_cast<unsigned long long>(cons2->get_value()));
                                }
                                tokens.erase(tokens.begin() + (i+1), tokens.begin() + (i+2));
                                cons = cons2;
                            }
                            else if(cons->get_name() == PLUS && op->is_arithmetic()) {
                                tokens.erase(tokens.begin() + (i+1), tokens.begin() + (i+2));
                                cons = std::get<token*>(tokens[i+1]);
                            }
                            //g->print_group();
                            has_ops = true;
                            break;
                        }
                    }
                }
            }
            if(cons != nullptr && op != nullptr) {
                if(ant == nullptr) {
                    ant = invalid_ant;
                }
                //std::cout << "all valid" << std::endl;

                if(ant->get_name() == INT && cons->get_name() == INT) {
                    //std::cout << "integer" << std::endl;
                    int val = 0;
                    int ant_val = std::any_cast<int>(ant->get_value());
                    int cons_val = std::any_cast<int>(cons->get_value());
                    if(op->get_name() == EXPONENT) {
                        val = std::pow<int>(ant_val, cons_val);
                    }
                    else if(op->get_name() == STAR) {
                        val = ant_val * cons_val;
                    }
                    else if(op->get_name() == SLASH) {
                        val = ant_val / cons_val;
                    }
                    else if(op->get_name() == PLUS) {
                        //std::cout << "adding: " << ant_val << " " << cons_val << std::endl;
                        val = ant_val + cons_val;
                    }
                    else if(op->get_name() == MINUS) {
                        val = ant_val - cons_val;
                    }
                    //std::cout << "val: " << val << std::endl;
                    if(ant_index == -1) {
                        tokens.erase(tokens.begin(), tokens.begin() + 2);
                        tokens.insert(tokens.begin(), new token(INT, "INT", 0, val));
                    }
                    else {
                        tokens.erase(tokens.begin() + (ant_index), tokens.begin() + (ant_index+3));
                        tokens.insert(tokens.begin() + ant_index, new token(INT, "INT", 0, val));

                    }
                    g->type = INT;
                    g->value = val;
                }
            }
        }
    }
};

#endif //ARITHMETIC_EVALUATOR_H
