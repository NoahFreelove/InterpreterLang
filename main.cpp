#include <iostream>
#include "lang/lang.h"
#include "lang/memory/macro.h"

using token_element = std::variant<std::shared_ptr<token>, std::shared_ptr<token_group>>;
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

int main()
{
    //lang::interpreter::read_from_file("file.lang");
    lang::interpreter::init();

    auto t1 = std::make_shared<token>(IDENTIFIER, "x");
    auto t2 = std::make_shared<token>(PLUS, "+");
    auto t3 = std::make_shared<token>(INT, "2", 0, 2);
    auto t4 = std::make_shared<token>(STAR, "*");
    auto t5 = std::make_shared<token>(LEFT_PAREN, "(");
    auto t6 = std::make_shared<token>(IDENTIFIER, "y");
    auto t7 = std::make_shared<token>(RIGHT_PAREN, ")");

    auto mac = new macro({"x", "y"}, {t1, t2, t3, t4, t5, t6, t7});

    auto g1 = std::make_shared<token_group>();
    g1->tokens.push_back(std::make_shared<token_element>(std::make_shared<token>(FLOAT, "2", 0, 2.5f)));
    g1->tokens.push_back(std::make_shared<token_element>(std::make_shared<token>(STAR, "*")));

    g1->tokens.push_back(std::make_shared<token_element>(std::make_shared<token>(LEFT_PAREN, "(")));

    g1->tokens.push_back(std::make_shared<token_element>(std::make_shared<token>(INT, "5", 0, 5)));
    g1->tokens.push_back(std::make_shared<token_element>(std::make_shared<token>(PLUS, "+" )));
    g1->tokens.push_back(std::make_shared<token_element>(std::make_shared<token>(INT, "2", 0, 2)));

    g1->tokens.push_back(std::make_shared<token_element>(std::make_shared<token>(RIGHT_PAREN, ")")));

    auto g2 = std::make_shared<token_group>();
    g2->tokens.push_back(std::make_shared<token_element>(std::make_shared<token>(INT, "3", 0, 3)));

    /*auto result_group = mac.convert_input_to_macro({g1,g2});

    std::cout << "result type: " << id_to_name(result_group->type) << std::endl;
    if(result_group->type == INT) {
        std::cout << "Val: " << std::any_cast<int>(result_group->value) << std::endl;
    }*/
    lang::interpreter::macros->insert({"add_mul2", mac});
    lang::interpreter::input_loop();
    return 0;
}
