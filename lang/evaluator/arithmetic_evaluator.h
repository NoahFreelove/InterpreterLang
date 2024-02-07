#ifndef ARITHMETIC_EVALUATOR_H
#define ARITHMETIC_EVALUATOR_H
#include "../tokenizer/token_group.h"
#include "../tokenizer/token.h"
class arithmetic_evaluator {
public:
    using token_element = std::variant<token*, token_group*>;
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
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
            if(i == 0 && curr->is_arithmetic()) {
                lang::interpreter::error("operator with no antecedent");
                return false;
            }
            if(last_token != nullptr) {
                if(last_token->is_arithmetic() && curr->is_arithmetic()) {
                    lang::interpreter::error("two operators side by side");
                    return false;
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

        std::vector<token_element> tokens = g->tokens;

        for (int i = 0; i < tokens.size(); ++i) {
            if(is_group(tokens[i])) {
                token_group* tg = std::get<token_group*>(tokens[i]);
                if(tg->type == UNDETERMINED) {
                    recursive_evaluation(tg);
                }
                if(tg->type == INT) {
                    g->tokens[i] = new token(INT, "INT", 0, static_cast<int *>(tg->value));
                }
                if(tg->type == FLOAT) {
                    g->tokens[i] = new token(FLOAT, "FLOAT", 0, static_cast<float *>(tg->value));
                }
                if(tg->type == DOUBLE) {
                    g->tokens[i] = new token(DOUBLE, "DOUBLE", 0, static_cast<double *>(tg->value));
                }
                if(tg->type == LONG) {
                    g->tokens[i] = new token(LONG, "LONG", 0, static_cast<long *>(tg->value));
                }
                if(tg->type == ULONG64) {
                    g->tokens[i] = new token(ULONG64, "ULONG64", 0, static_cast<unsigned long long *>(tg->value));
                }
            }
        }

        // all parenthesis are gone, all groups evaluated, just eval.
        bool has_ops = true;
        if(!check_errs(tokens))
            return;

        while (has_ops) {
            has_ops = false;

        }
    }
};

#endif //ARITHMETIC_EVALUATOR_H
