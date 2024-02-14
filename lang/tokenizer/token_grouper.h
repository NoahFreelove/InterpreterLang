#ifndef TOKEN_GROUPER_H
#define TOKEN_GROUPER_H
#include <stack>

#include "token.h"
#include "token_group.h"
#include "../interpreter.h"

class token_grouper {
public:
    static std::shared_ptr<token_group> proc_group(std::vector<std::shared_ptr<token>> tokens, int index) {
        for (int i = index; i < tokens.size(); ++i) {

        }
        return std::make_shared<token_group>();
    }
    static std::shared_ptr<token_group> recursive_group(std::vector<std::shared_ptr<token>> tokens) {
        // brackets in the form of ( and ) are used to group tokens
        // so (1+2 + 3*(4+5)) would be grouped as
        // (1,+,2,+,3,*),(4,+,5)
        // nothing can be on its own unless its in a group
        // recursive function to group tokens
        // everything before the first bracket is in the first group
        // if a bracket is found, everything in it and not including the bracket itself is in a new group
        // make sure to count num brackets to ensure the correct group is found
        // everything after the bracket is in its own group until the next bracket is found
        // repeat until no more brackets are found
        // if(token.get_name() == LEFT_PAREN)
        // if(token.get_name() == RIGHT_PAREN)
        //
        std::stack<std::shared_ptr<token_group>> groups;
        auto root_group = std::make_shared<token_group>(); // This will be the root group
        groups.push(root_group); // Start with the root group on the stack
        int i = 0;
        for (auto tk : tokens) {
            int type = tk->get_name();

            if (type == LEFT_PAREN) {
                // Start a new group for everything inside the parentheses
                auto new_group = std::make_shared<token_group>();
                std::shared_ptr<token_group::token_element> element = std::make_shared<token_group::token_element>(new_group);

                groups.top()->add(element); // Add this new group to the current group
                groups.push(new_group); // Make this the current group
            } else if (type == RIGHT_PAREN) {
                // End the current group and go back to the parent group
                groups.pop(); // Pop without adding, it's already added when LEFT_PAREN was encountered
                if (groups.empty()) {
                    // Error handling: encountered a closing parenthesis without a matching opening one
                    return std::make_shared<token_group>(); // Return an empty group or handle error
                }
            }
            else {
                // Add the token to the current group
                std::shared_ptr<token_group::token_element> element = std::make_shared<token_group::token_element>(tk);
                groups.top()->add(element);
            }
            i++;
        }

        if (groups.size() != 1) {
            // Error handling: not all groups were properly closed
            return std::make_shared<token_group>(); // Return an empty group or handle error
        }

        // At this point, `root_group` contains all tokens properly grouped
        return root_group; // Return the root group
    }
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
    static std::shared_ptr<token_group> gen_group(std::vector<std::shared_ptr<token>> tokens) {
        generate_parens(tokens);

        // print them out
        /*for (auto tk : tokens) {
            std::cout << tk->get_lexeme() << " ";
        }
        std::cout << std::endl;*/

        return recursive_group(tokens);

    }
};
#endif //TOKEN_GROUPER_H
