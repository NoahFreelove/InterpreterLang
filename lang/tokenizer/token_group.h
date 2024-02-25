#ifndef TOKEN_GROUP_H
#define TOKEN_GROUP_H
#include <variant>
#include <vector>
#include <iostream>
#include <memory>
#include "id_to_name.h"
#include "token.h"

// These represent the evaluation type of the group
#define UNDETERMINED 1
#define NOTHING (-2)
#define ERROR (-1)

class token_group {
public:
    using token_element = std::variant<std::shared_ptr<token>, std::shared_ptr<token_group>>;
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    int type = UNDETERMINED;
    std::any value = nullptr;

    std::vector<std::shared_ptr<token_element>> tokens; // can store tokens or token groups
    token_group() {
        tokens = {};
    }
    token_group(int type, const std::any &value) {
        this->type = type;
        this->value = value;
    }

    void add(std::shared_ptr<token_element> item) {
        tokens.push_back(item);
    }

    void output_group(std::string& curr_out, int depth = 0) {
        if(depth == 0)
            curr_out = "";

        for (const std::shared_ptr<token_element>& element : tokens) {
            std::visit(overloaded{
                [&curr_out](const std::shared_ptr<token>& tk) {

               if(tk->get_value().type() == typeid(int)) {
                   curr_out += std::to_string(std::any_cast<int>(tk->get_value()));
               } else if(tk->get_value().type() == typeid(float)) {
                   curr_out += std::to_string(std::any_cast<float>(tk->get_value()));
               } else if(tk->get_value().type() == typeid(double)) {
                   curr_out += std::to_string(std::any_cast<double>(tk->get_value()));
               }
               else if(tk->get_value().type() == typeid(long)) {
                   curr_out += std::to_string(std::any_cast<long>(tk->get_value()));
               }
               else if(tk->get_value().type() == typeid(unsigned long long)) {
                curr_out += std::to_string(std::any_cast<unsigned long long>(tk->get_value()));
               }
               else {
                   curr_out += id_to_name(tk->get_name());
               }
                },
                [depth, &curr_out](const std::shared_ptr<token_group>& grp) {
                    curr_out += "[";
                    grp->output_group(curr_out, depth + 1);
                    curr_out += "]";

                }
            }, *element);
        }
    }

    void print_group(int depth = 0) {
        for (const std::shared_ptr<token_element>& element : tokens) {
            std::visit(overloaded{
                [](const std::shared_ptr<token>& tk) {

               if(tk->get_value().type() == typeid(int)) {
                   std::cout << std::any_cast<int>(tk->get_value());
               } else if(tk->get_value().type() == typeid(float)) {
                   std::cout << std::any_cast<float>(tk->get_value());
               } else if(tk->get_value().type() == typeid(double)) {
                   std::cout << std::any_cast<double>(tk->get_value());
               }
               else if(tk->get_value().type() == typeid(long)) {
                   std::cout << std::any_cast<long>(tk->get_value());
               }
               else {
                   std::cout << id_to_name(tk->get_name());
               }
                },
                [depth](const std::shared_ptr<token_group>& grp) {
                    std::cout << "[";
                    grp->print_group(depth + 1);
                    std::cout << "]";

                }
            }, *element);
        }
        if(depth == 0) {
            std::cout << std::endl;
        }
    }
};
#endif //TOKEN_GROUP_H
