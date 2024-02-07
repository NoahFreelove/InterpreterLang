#ifndef TOKEN_GROUP_H
#define TOKEN_GROUP_H
#include <variant>
#include <vector>
#include "id_to_name.h"

// These represent the evaluation type of the group
#define UNDETERMINED 1

class token_group {
public:
    using token_element = std::variant<token*, token_group*>;
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    int type = UNDETERMINED;
    std::any value = nullptr;

    ~token_group() {
        for (const token_element& element : tokens) {
            std::visit(overloaded{
                [](token* tk) {
                    delete tk;
                },
                [](token_group* grp) {
                    delete grp;
                }
            }, element);
        }
    }

    std::vector<token_element> tokens; // can store tokens or token groups
    token_group() {
        tokens = {};
    }

    void add(token_element item) {
        tokens.push_back(item);
    }

    void print_group(int depth = 0) {
        for (const token_element& element : tokens) {
            std::visit(overloaded{
                [](token* tk) {

               if(tk->get_value().type() == typeid(int)) {
                   std::cout << std::any_cast<int>(tk->get_value());
               } else if(tk->get_value().type() == typeid(float)) {
                   std::cout << std::any_cast<float>(tk->get_value());
               } else if(tk->get_value().type() == typeid(double)) {
                   std::cout << std::any_cast<double>(tk->get_value());
               } else {
                   std::cout << id_to_name(tk->get_name());
               }

                },
                [depth](token_group* grp) {
                    std::cout << "[";
                    grp->print_group(depth + 1);
                    std::cout << "]";

                }
            }, element);
        }
        if(depth == 0) {
            std::cout << std::endl;
        }
    }
};
#endif //TOKEN_GROUP_H
