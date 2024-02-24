#ifndef TOKEN_GROUPER_H
#define TOKEN_GROUPER_H
#include <stack>

#include "token.h"
#include "token_group.h"
#include "../interpreter.h"
#include "../memory/stack_manager.h"
class token_grouper {
public:
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    static std::shared_ptr<token_group> recursive_group(std::vector<std::shared_ptr<token>>& tokens);
    static void generate_parens(std::vector<std::shared_ptr<token>>& tokens) {
        // if there is a truthy check: == aka. DEQUAL,
        // we want to put parenthesis around the antecedent and concequents
        // so: 5-5 == false -> (5-5) == (false)
        // if there is a truthy check: && aka. AND,
        // we want to put parenthesis around the antecedent and concequents
        // so: 5-5 && false -> (5-5) && (false)
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (tokens[i]->is_logical()) {
                if(i == 0) {
                    lang::interpreter::error("Cannot start an expression with a binary operator");
                    return;
                }
                if(i == tokens.size() - 1) {
                    lang::interpreter::error("Cannot end an expression with a binary operator");
                    return;
                }
                tokens.insert(tokens.begin() + i, std::make_unique<token>(RIGHT_PAREN, ")",0,0));
                i++;
                tokens.insert(tokens.begin() + i+1, std::make_unique<token>(LEFT_PAREN, "(",0,0));
                i++;
            }
            else {
                if(i == 0) {
                    tokens.insert(tokens.begin(), std::make_unique<token>(LEFT_PAREN, "(",0,0));
                    i++;
                }
            }
        }
        tokens.insert(tokens.end(), std::make_unique<token>(RIGHT_PAREN, ")",0,0));
    }
    static std::shared_ptr<token_group> gen_group(std::vector<std::shared_ptr<token>> tokens);

    static std::shared_ptr<token_group> recursive_clone_group(const std::shared_ptr<token_group>& group) {
        auto new_group = std::make_shared<token_group>();
        for (const auto& tk : group->tokens) {
            std::visit(overloaded{
                [&new_group](const std::shared_ptr<token>& tk) {
                    new_group->add(std::make_shared<token_group::token_element>(std::make_shared<token>(tk.get())));
                },
                [&new_group](const std::shared_ptr<token_group>& grp) {
                    new_group->add(std::make_shared<token_group::token_element>(recursive_clone_group(grp)));
                }
            }, *tk);
        }
        return new_group;
    }
};
#endif //TOKEN_GROUPER_H
