#ifndef TRUTHY_EVALUATOR_H
#define TRUTHY_EVALUATOR_H
#include "../tokenizer/token_group.h"
#include "../tokenizer/token.h"
class truthy_evaluator {
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

    static std::shared_ptr<token_element> convert(int name, const char* lexeme, int line, std::any value) {
        return std::make_shared<token_element>(std::make_shared<token>(name, lexeme, 0, value));
    }
    inline static std::shared_ptr<token> invalid_ant = std::make_shared<token>(INT, "INT", 0, 0);

    static int get_highest_level_op(std::shared_ptr<token_group> g) {
        int highest = 0;
        for (const auto& t : g->tokens) {
            if(!is_group(*t)) {
                auto tk = std::get<std::shared_ptr<token>>(*t);
                if(tk->is_logical()) {
                    if (tk->get_name() == AND || tk->get_name() == OR || tk->get_name() == XOR) {
                        highest = std::max(highest, 2);
                    }
                    else {
                        highest = std::max(highest,1);
                    }
                }
            }
        }
        return highest;
    }

    static double get_double_value(int name, std::any value) {
        if(name == INT) {
            return std::any_cast<int>(value);
        }
        if(name == LONG) {
            return std::any_cast<long>(value);
        }
        if(name == FLOAT) {
            return std::any_cast<float>(value);
        }
        if(name == DOUBLE) {
            return std::any_cast<double>(value);
        }
        if(name == TRUE) {
            return 1;
        }
        if(name == FALSE) {
            return 0;
        }
        return 0;
    }

    static void recursive_evaluation(std::shared_ptr<token_group>& g) {
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
                    else if(tg->type == TRUE) {
                        g->tokens[i] = convert(TRUE, "TRUE", 0, true);
                    }
                    else if(tg->type == FALSE) {
                        g->tokens[i] = convert(FALSE, "FALSE", 0, false);
                    }
                    if(tg->type == INT || tg->type == FLOAT || tg->type == DOUBLE || tg->type == LONG) {
                        g->tokens[i] = convert(tg->type, "NUMBER", 0, tg->value);

                    }
                }
            }
        }
        // if there is a single true or false token, just set this groups' value to that
        if (tokens.size() == 1) {
            auto t = std::get<std::shared_ptr<token>>(*tokens[0]);
            if(t->get_name() == TRUE) {
                g->type = TRUE;
                g->value = true;
            }
            else if(t->get_name() == FALSE) {
                g->type = FALSE;
                g->value = false;
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
                    // do and and or first because they have higher precedence over ==, > ops
                    if(t->get_name() == AND || t->get_name() == OR || t->get_name() == XOR) {
                        ant_index = i - 1;
                        ant = std::get<std::shared_ptr<token>>(*tokens[ant_index]);
                        cons = std::get<std::shared_ptr<token>>(*tokens[i + 1]);
                        op = t;
                        has_ops = true;
                        break;
                    }
                    else if(get_highest_level_op(g) == 1) {
                        ant_index = i - 1;
                        ant = std::get<std::shared_ptr<token>>(*tokens[ant_index]);
                        cons = std::get<std::shared_ptr<token>>(*tokens[i + 1]);
                        op = t;
                        has_ops = true;
                        break;
                    }
                }
            }
            if(cons != nullptr && op != nullptr) {
                if(ant == nullptr) {
                        ant = invalid_ant;
                    }
                    int type = INT;
                    const char* type_str = "";
                    std::any val;
                    std::cout << id_to_name(op->get_name()) << std:: endl;
                bool result = false;
                if(op->is_first_order_logic()) {
                    bool a = (ant->get_name() == TRUE);
                    bool b = (cons->get_name() == TRUE);
                    if(op->get_name() == XOR) {
                        result = (a && !b) || (b && !a);
                    }
                    else if(op->get_name() == OR) {
                        result = a || b;
                    }
                    else if(op->get_name() == AND) {
                        result = a&&b;
                    }
                }
                else
                    {
                    double a = get_double_value(ant->get_name(), ant->get_value());
                    double b = get_double_value(cons->get_name(), cons->get_value());
                    //std::cout << "a: " << a << std::endl;
                    //std::cout << "b: " << b << std::endl;


                    if(op->get_name() == GT) {
                        result = a>b;
                    }
                    else if(op->get_name() == LT) {
                        result = a<b;
                    }
                    else if(op->get_name() == LTE) {
                        result = a<=b;
                    }
                    else if(op->get_name() == GTE) {
                        result = a>=b;
                    }
                    else if(op->get_name() == DEQUAL) {
                        result = a==b;
                    }
                    else if(op->get_name() == BANG_EQUAL) {
                        result = a!=b;
                    }
                    else if(op->get_name() == TEQUAL) {
                        if(ant->get_name() != cons->get_name())
                            result = false;
                        else
                            result = a==b;
                    }
                }

                    if(result) {
                        type = TRUE;
                        type_str = "TRUE";
                    }
                    else {
                        type = FALSE;
                        type_str = "FALSE";
                    }

                    if(ant_index == -1) {
                        tokens.erase(tokens.begin(), tokens.begin() + 2);
                        tokens.insert(tokens.begin(), convert(type,type_str,0,result));
                    }
                    else {
                        tokens.erase(tokens.begin() + (ant_index), tokens.begin() + (ant_index+3));
                        tokens.insert(tokens.begin() + ant_index, convert(type,type_str,0,result));
                    }
                    g->type = type;
                    g->value = result;
            }
        }

    }
public:
    static void truth_eval(std::shared_ptr<token_group>& g) {
        recursive_evaluation(g);
    }
};
#endif //TRUTHY_EVALUATOR_H
