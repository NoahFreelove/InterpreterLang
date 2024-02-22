#ifndef ARITHMETIC_EVALUATOR_H
#define ARITHMETIC_EVALUATOR_H
#include <complex>

#include "../tokenizer/token_group.h"
#include "../tokenizer/token.h"
#include "type_arithmetic.h"
#include <cstring>
class arithmetic_evaluator {
public:
    using token_element = std::variant<std::shared_ptr<token>, std::shared_ptr<token_group>>;
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;


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
    static bool has_next(std::shared_ptr<token_group> g, int i, int inc) {
        return (i+inc) < g->tokens.size();
    }
    static std::shared_ptr<token_element> convert(int name, const char* lexeme, int line, std::any value) {
        return std::make_shared<token_element>(std::make_shared<token>(name, lexeme, 0, value));
    }
    inline static std::shared_ptr<token> invalid_ant = std::make_shared<token>(INT, "INT", 0, 0);

    static bool check_errs(std::vector<std::shared_ptr<token_element>> tokens) {
        std::shared_ptr<token> last_token = nullptr;
        for (int i = 0; i < tokens.size(); ++i) {
            if(is_group(*tokens[1]))
                continue;
            auto curr = std::get<std::shared_ptr<token>>(*tokens[i]);
            //std::cout << *curr << std::endl;
            if(last_token != nullptr) {
                //std::cout << "last: " << *last_token << std::endl;
            }
            if(i == 0 && curr->is_arithmetic() && !curr->is_add_sub()) {
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
                // Now we convert truthy to 1 or 0
                /*if(last_token->is_arithmetic() && curr->is_truthy()) {
                    lang::interpreter::error("arithmetic operator followed by truthy value");
                    return false;
                }*/
                if(last_token->is_arithmetic() && curr->is_builtin()) {
                    lang::interpreter::error("arithmetic operator followed by builtin function");
                    return false;
                }
                last_token = curr;
            }
            else {
                last_token = curr;
            }
        }
        return true;
    }

    static int get_highest_level_op(std::shared_ptr<token_group> g) {
        int highest = 0;
        for (const auto& t : g->tokens) {
            if(!is_group(*t)) {
                auto tk = std::get<std::shared_ptr<token>>(*t);
                if(tk->is_arithmetic()) {
                    if (tk->is_mul_div()) {
                        highest = std::max(highest, 1);
                    }
                    if (tk->get_name() == EXPONENT) {
                        highest = std::max(highest, 2);
                    }
                }
            }
        }
        return highest;
    }

    static void recursive_evaluation(const std::shared_ptr<token_group>& g, int depth = 0) {
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

        if(depth >= lang::interpreter::max_depth) {
            auto out = std::string("Max arithmetic depth reached: " + depth);
            out += " consider making your arithmetic simpler, or an internal language error occured.";
            lang::interpreter::error(out);
            g->type = ERROR;
            return;
        }

        std::vector<std::shared_ptr<token_element>>& tokens = g->tokens;
        bool groups_found = true;
        while (groups_found) {
            groups_found = false;
            for (int i = 0; i < tokens.size(); ++i) {
                if(is_group(*tokens[i])) {
                    groups_found = true;
                    auto tg = std::get<std::shared_ptr<token_group>>(*tokens[i]);
                    if(tg->type == UNDETERMINED) {
                        recursive_evaluation(tg, depth+1);
                    }
                    if(tg->type == INT) {
                        g->tokens[i] = convert(INT, "INT", 0, tg->value);
                    }
                    if(tg->type == FLOAT) {
                        g->tokens[i] = convert(FLOAT, "FLOAT", 0, tg->value);
                    }
                    if(tg->type == DOUBLE) {
                        g->tokens[i] = convert(DOUBLE, "DOUBLE", 0, tg->value);
                    }
                    if(tg->type == LONG) {
                        g->tokens[i] = convert(LONG, "LONG", 0, tg->value);
                    }
                    if(tg->type == ULONG64) {
                        g->tokens[i] = convert(ULONG64, "ULONG64", 0, tg->value);
                    }
                    if(tg->type == TRUE || tg->type == FALSE){
                        g->tokens[i] = convert(INT, "INT", 0, int(tg->type == TRUE));
                    }
                    if(tg->type == ERROR || tg->type == NOTHING) {
                        groups_found = false;
                        continue;
                    }
                }
            }
        }

        if(g->tokens.size() == 1) {
            if (!is_group(*g->tokens[0])) {
                g->type = g->type;
                g->value = std::get<std::shared_ptr<token>>(*g->tokens[0])->get_value();
                return;
            }
        }

        // all parenthesis are gone, all groups evaluated, just eval.
        if(!check_errs(tokens))
            return;

        bool has_ops = true;
        int highest_level_op = 0;
        // 0 - add/sub, 1 - mult/div, 2 - exponent
        while (has_ops) {
            //g->print_group();
            has_ops = false;
            // find antecedent and consequent of any operators
            // Do one pass to find * or /. If they are found find the antecedent and concequent
            // If these aren't found, find the first of + or -
            // Replace the op, ant, and cons with a new token of the arithmetic value
            int ant_index = -1;
            std::shared_ptr<token> ant = nullptr;
            std::shared_ptr<token> cons = nullptr;
            std::shared_ptr<token> op = nullptr;
            for (int i = 0; i < tokens.size(); ++i) {
                std::shared_ptr<token> t = nullptr;
                // if its an instance of group, cout it
                if(is_group(*tokens[i])) {
                    std::cout << "GROUP" << std::endl;
                    return;
                }
                else {
                    t = std::get<std::shared_ptr<token>>(*tokens[i]);
                }
                highest_level_op = get_highest_level_op(g);
                //std::cout << "highest op: " << highest_level_op << std::endl;

                //g->print_group();

                if(t->is_arithmetic()) {
                    if(t->get_name() == EXPONENT) {
                        if(has_next(g, i, 1)) {
                            ant_index = i-1;
                            ant = std::get<std::shared_ptr<token>>(*tokens[i-1]);
                            cons = std::get<std::shared_ptr<token>>(*tokens[i+1]);
                            op = std::get<std::shared_ptr<token>>(*tokens[i]);
                            has_ops = true;
                            break;
                        }
                    }
                    if( highest_level_op == 1 && (t->is_mul_div())) {
                        //std::cout << "star or slash" << std::endl;
                        if(has_next(g, i, 1)) {
                            ant_index = i-1;
                            ant = std::get<std::shared_ptr<token>>(*tokens[i-1]);
                            cons = std::get<std::shared_ptr<token>>(*tokens[i+1]);
                            op = std::get<std::shared_ptr<token>>(*tokens[i]);
                            has_ops = true;
                            break;
                        }
                    }
                    if(highest_level_op == 0 && (t->get_name() == PLUS || t->get_name() == MINUS)) {
                        //std::cout << "add or sub" << std::endl;

                        if(has_next(g, i, 1)) {
                            ant_index = i-1;
                            if(ant_index < 0) {
                                ant = nullptr;
                            }
                            else
                                ant = std::get<std::shared_ptr<token>>(*tokens[i-1]);

                            op = std::get<std::shared_ptr<token>>(*tokens[i]);
                            cons = std::get<std::shared_ptr<token>>(*tokens[i+1]);
                            // if cons is a minus, take the value after it, and negate it, then replace the cons with the new value
                            // after the minus and delete the minus so its just an addition
                            if(cons->get_name() == MINUS && op->is_arithmetic()) {
                                //g->print_group();
                                //std::cout << "replacing" << std::endl;
                                if(!has_next(g,i,2))
                                    continue;
                                std::shared_ptr<token> cons2 = std::get<std::shared_ptr<token>>(*tokens[i+2]);
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
                                cons = std::get<std::shared_ptr<token>>(*tokens[i+1]);
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

                if(ant->get_name() == TRUE || ant->get_name() == FALSE) {
                    ant = std::make_shared<token>(INT, "INT",0, int(ant->get_name() == TRUE));
                }
                if(cons->get_name() == TRUE || cons->get_name() == FALSE) {
                    cons = std::make_shared<token>(INT, "INT",0, int(cons->get_name() == TRUE));
                }

                if((!cons->is_numeric() || !ant->is_numeric()) && (cons->get_name() != STRING && ant->get_name() != STRING)){
                    lang::interpreter::error("non-numeric/string value in arithmetic operation");
                    return;
                }
                //std::cout << "all valid" << std::endl;
                // if ant or cons is a string and op is add, cast the non-string to a string and add them together
                if(ant->get_name() == STRING && op->get_name() == PLUS && cons->is_numeric()) {
                    std::shared_ptr<token> old = cons;
                    std::string cons_str = cons->to_string();
                    tokens.erase(tokens.begin() + (ant_index+2), tokens.begin() + (ant_index+3));
                    const char* cpy = (const char*)malloc(cons_str.size() + 1);
                    strcpy((char*)cpy, cons_str.c_str());
                    std::shared_ptr<token> new_cons = std::make_shared<token>(STRING, cpy, 0, cons_str);
                    cons = new_cons;
                    tokens.insert(tokens.begin() + ant_index, std::make_shared<token_element>(cons));

                }
                else if(cons->get_name() == STRING && op->get_name() == PLUS && ant->is_numeric()) {
                    std::shared_ptr<token> old = ant;
                    std::string ant_str = ant->to_string();
                    tokens.erase(tokens.begin() + (ant_index), tokens.begin() + (ant_index+1));
                    const char* cpy = (const char*)malloc(ant_str.size() + 1);
                    strcpy((char*)cpy, ant_str.c_str());
                    std::shared_ptr<token> new_ant = std::make_shared<token>(STRING, cpy, 0, ant_str);
                    ant = new_ant;
                    tokens.insert(tokens.begin() + ant_index,std::make_shared<token_element>(ant));
                }

                if(ant->get_name() == STRING && cons->get_name() == STRING && op->get_name() == PLUS) {
                    std::string val = std::any_cast<std::string>(ant->get_value()) + std::any_cast<std::string>(cons->get_value());
                    if(ant_index == -1) {
                        continue;
                    }
                    else {
                        tokens.erase(tokens.begin() + (ant_index), tokens.begin() + (ant_index+3));
                        const char* cpy = (const char*)malloc(val.size() + 1);
                        std::strcpy((char*)cpy, val.c_str());
                        tokens.insert(tokens.begin() + ant_index, convert(STRING, cpy, 0, val));
                        g->type = STRING;
                        g->value = val;
                    }
                }
                if(!ant->is_numeric() || !cons->is_numeric())
                    return;
                int type = INT;
                const char* type_str = "";
                std::any val;

                // Logic for adding different types, the larger one takes precedent

                if((ant->get_name() == DOUBLE || cons->get_name() == DOUBLE) && (cons->is_DFI() && ant->is_DFI())) {
                    double ant_val = 0;
                    double cons_val = 0;

                    if(ant->get_name() == DOUBLE) {
                        ant_val = std::any_cast<double>(ant->get_value());
                    }
                    else if(ant->get_name() == FLOAT) {
                        ant_val = std::any_cast<float>(ant->get_value());
                    }
                    if(ant->get_name() == INT) {
                        ant_val = std::any_cast<int>(ant->get_value());
                    }

                    if(cons->get_name() == DOUBLE) {
                        cons_val = std::any_cast<double>(cons->get_value());
                    }
                    else if(cons->get_name() == FLOAT) {
                        cons_val = std::any_cast<float>(cons->get_value());
                    }
                    if(cons->get_name() == INT) {
                        cons_val = std::any_cast<int>(cons->get_value());
                    }

                    val = type_arithmetic::result(ant_val, cons_val, op->get_name());
                    type = DOUBLE;
                    type_str = "DOUBLE";
                }

                if((ant->get_name() == FLOAT || cons->get_name() == FLOAT) && (cons->is_FI() && ant->is_FI())) {
                    float ant_val = 0;
                    float cons_val = 0;

                    if(ant->get_name() == FLOAT) {
                        ant_val = std::any_cast<float>(ant->get_value());
                    }
                    if(ant->get_name() == INT) {
                        ant_val = std::any_cast<int>(ant->get_value());
                    }

                   if(cons->get_name() == FLOAT) {
                        cons_val = std::any_cast<float>(cons->get_value());
                    }
                    if(cons->get_name() == INT) {
                        cons_val = std::any_cast<int>(cons->get_value());
                    }

                    val = type_arithmetic::result(ant_val, cons_val, op->get_name());
                    type = FLOAT;
                    type_str = "FLOAT";
                }

                if((ant->get_name() == LONG || cons->get_name() == LONG) && (cons->is_LI() && ant->is_LI())) {
                    long ant_val = 0;
                    long cons_val = 0;

                    if(ant->get_name() == LONG) {
                        ant_val = std::any_cast<long>(ant->get_value());
                    }
                    if(ant->get_name() == INT) {
                        ant_val = std::any_cast<int>(ant->get_value());
                    }
                    if(cons->get_name() == LONG) {
                        cons_val = std::any_cast<long>(cons->get_value());
                    }
                    if(cons->get_name() == INT) {
                        cons_val = std::any_cast<int>(cons->get_value());
                    }

                    val = type_arithmetic::result(ant_val, cons_val, op->get_name());
                    type = LONG;
                    type_str = "LONG";
                }

                if(ant->get_name() == INT && cons->get_name() == INT) {
                    int ant_val = std::any_cast<int>(ant->get_value());
                    int cons_val = std::any_cast<int>(cons->get_value());
                    val = type_arithmetic::result(ant_val, cons_val, op->get_name());
                    type = INT;
                    type_str = "INT";
                }

                if(ant_index == -1) {
                    tokens.erase(tokens.begin(), tokens.begin() + 2);
                    tokens.insert(tokens.begin(), convert(type,type_str,0,val));
                }
                else {
                    tokens.erase(tokens.begin() + (ant_index), tokens.begin() + (ant_index+3));
                    tokens.insert(tokens.begin() + ant_index, convert(type,type_str,0,val));
                }
                g->type = type;
                g->value = val;
            }
        }
    }

    static void convert_op_eq_to_op(std::vector<std::shared_ptr<token>>& tokens) {
        // get op from second token and save it to some variable
        // delete it from vector
        // get identifier from first token copy it.
        // insert the identifier, then the op, then an open parenthesis after the new second token which should be an equal sign
        // finally, insert a close parenthesis at the very end of tokens
        std::shared_ptr<token> op = tokens[1];
        tokens.erase(tokens.begin() + 1);
        std::shared_ptr<token> id = tokens[0];
        tokens.insert(tokens.begin() + 2, std::make_shared<token>(IDENTIFIER, id->get_lexeme(), 0, id->get_value()));
        tokens.insert(tokens.begin() + 3, op);
        tokens.insert(tokens.begin() + 4, std::make_shared<token>(LEFT_PAREN, "(", 0, 0));
        tokens.push_back(std::make_shared<token>(RIGHT_PAREN, ")", 0, 0));
        /*for (int i = 0; i < tokens.size(); ++i) {
            std::cout << *tokens[i] << std::endl;
        }*/
    }
    static void convert_inc_to_op(std::vector<std::shared_ptr<token>> & tokens) {
        auto id = tokens[0];
        // erase all in tokens
        tokens.clear();
        tokens.push_back(id);
        tokens.push_back(std::make_shared<token>(EQUAL, "=", 0, 0));
        tokens.push_back(std::make_shared<token>(IDENTIFIER, id->get_lexeme(), 0, id->get_lexeme()));
        tokens.push_back(std::make_shared<token>(PLUS, "+", 0, 0));
        tokens.push_back(std::make_shared<token>(INT, "1", 0, 1));
    }

    static void convert_dec_to_op(std::vector<std::shared_ptr<token>> & tokens) {
        auto id = tokens[0];
        // erase all in tokens
        tokens.clear();
        tokens.push_back(id);
        tokens.push_back(std::make_shared<token>(EQUAL, "=", 0, 0));
        tokens.push_back(std::make_shared<token>(IDENTIFIER, id->get_lexeme(), 0, id->get_lexeme()));
        tokens.push_back(std::make_shared<token>(MINUS, "-", 0, 0));
        tokens.push_back(std::make_shared<token>(INT, "1", 0, 1));
    }
};

#endif //ARITHMETIC_EVALUATOR_H
