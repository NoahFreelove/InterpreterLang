#ifndef TOKEN_GROUPER_H
#define TOKEN_GROUPER_H
#include <stack>

#include "token.h"
#include "token_group.h"
#include "../interpreter.h"
#include "../memory/stack_manager.h"
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
        for (int i = 0; i < tokens.size(); i++) {
            auto tk = tokens[i];
            int type = tk->get_name();
            if(type == IDENTIFIER && i+1 < tokens.size()) {
                if(tokens[i+1]->get_name() == LEFT_PAREN) {
                    auto* proc = resolve_proc(tk->get_lexeme());

                    if(proc) {
                        std::vector<std::vector<std::shared_ptr<token>>> arguments;
                        auto curr_arg = std::vector<std::shared_ptr<token>>();
                        int j = i+2;
                        int num_left_paren = 1;
                        while (tokens[j]->get_name() != RIGHT_PAREN || num_left_paren != 0) {
                            //std::cout << "TOKEN: " << *tokens[j] << std::endl;
                            if(tokens[j]->get_name() == RIGHT_PAREN) {
                                num_left_paren--;
                                // std::cout << "NUM LEFT PAREN: " << num_left_paren << std::endl;
                                if (num_left_paren == 0) {
                                    break;
                                }
                            }
                            if(j >= tokens.size()) {
                                lang::interpreter::error("Expected ')' in procedure call");
                                return std::make_shared<token_group>();
                            }
                            if(tokens[j]->get_name() == COMMA) {
                                // copy value of vector to arguments, so we can clear it
                                if(!curr_arg.empty()) {
                                    arguments.emplace_back(curr_arg);
                                }
                                else {
                                    std::cout << "Empty arg" << std::endl;
                                }
                                curr_arg.clear();
                            }
                            else {
                                if(tokens[j]->get_name() == LEFT_PAREN)
                                    num_left_paren++;
                                curr_arg.push_back(tokens[j]);
                            }
                            j++;
                        }
                        if(!curr_arg.empty()) {
                            arguments.emplace_back(curr_arg);
                        }
                        /*std::cout << "PRE PRINT:" << std::endl;
                        for (auto& tok : tokens) {
                            std::cout << *tok << std::endl;
                        }*/
                        // Erase tokens from (i,j]
                        tokens.erase(tokens.begin() + i+1, tokens.begin() + j+1);
                        /*std::cout << "AFTER ERASE: " << std::endl;
                        for (auto& tok : tokens) {
                            std::cout << *tok << std::endl;
                        }*/

                        std::vector<std::shared_ptr<token_group>> grouped_args;
                        grouped_args.reserve(arguments.size());
                        for(auto& vec : arguments) {
                            auto g = recursive_group(vec);
                            grouped_args.push_back(g);
                        }
                        auto proc_ptr = std::make_shared<token>(PROC, tk->get_lexeme(), 0, grouped_args);
                        tokens[i] = proc_ptr;
                        auto wrapper = std::make_shared<token_group>();
                        std::shared_ptr<token_group::token_element> element = std::make_shared<token_group::token_element>(proc_ptr);

                        wrapper->tokens.push_back(element);
                        groups.top()->tokens.push_back(std::make_shared<token_group::token_element>(wrapper));
                        continue;

                    }
                    else {
                        lang::interpreter::error("Expected procedure call, no procedure with name '" + std::string(tk->get_lexeme()) + "' was found.");
                        return std::make_shared<token_group>();
                    }
                }
            }

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
        /*for (const auto& tk : tokens) {
            std::cout << tk->get_lexeme() << " ";
        }
        std::cout << std::endl;*/

        auto g = recursive_group(tokens);
        return g;
    }
};
#endif //TOKEN_GROUPER_H
