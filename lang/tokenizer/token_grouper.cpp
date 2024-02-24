#include "token_grouper.h"
#include "../evaluator/group_evaluator.h"

std::shared_ptr<token_group> token_grouper::recursive_group(std::vector<std::shared_ptr<token>> &tokens)  {
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
        std::vector<std::shared_ptr<token_group>> groups;
        auto root_group = std::make_shared<token_group>(); // This will be the root group
        groups.push_back(root_group); // Start with the root group on the stack
        for (int i = 0; i < tokens.size(); i++) {
            auto tk = tokens[i];
            int type = tk->get_name();

            if(type == FORWARD) {
                if(i == 0 && groups.size() == 1) {
                    lang::interpreter::error("Forward keyword must be used on a token in the same group as the succeeding procedure");
                    return std::make_shared<token_group>();
                }
                else if(i+2 >= tokens.size()) {
                    lang::interpreter::error("Forward keyword must have a valid procedure after it");
                    return std::make_shared<token_group>();
                }
                if(tokens[i+1]->get_name() != IDENTIFIER || tokens[i+2]->get_name() != LEFT_PAREN) {
                    lang::interpreter::error("Forward keyword must have a valid procedure after it");
                    return std::make_shared<token_group>();
                }

                if(tokens[i-1]->get_name() != RIGHT_PAREN) {
                    //std::cout << "valid forward: not group" << std::endl;
                    tokens.insert(tokens.begin() + i + 3, tokens[i-1]);
                    groups.back()->tokens.pop_back();
                }
                else if(tokens[i-1]->get_name() == RIGHT_PAREN) {
                    if(!group_evaluator::is_group(*groups.back()->tokens.back())) {
                        lang::interpreter::error(" Right paren found but no group!");
                        return std::make_shared<token_group>();
                    }

                    auto prev = std::get<std::shared_ptr<token_group>>(*groups.back()->tokens.back());
                    std::vector<std::shared_ptr<token>> output;
                    group_evaluator::eval_group(prev);
                    if(prev->type == UNDETERMINED || prev->type == ERROR) {
                        lang::interpreter::error("Forward group evaluation error. Type: " + id_to_name(prev->type));
                        return std::make_shared<token_group>();
                    }
                    //std::cout << "valid forward: is group" << std::endl;
                    auto new_tok = std::make_shared<token>(prev->type, id_to_name(prev->type).c_str(), 0, prev->value);
                    tokens.insert(tokens.begin() + i + 3, new_tok);
                    //std::cout << "Groups size: " << groups.size() << std::endl;
                    groups.back()->tokens.pop_back();
                    /*for(int j = i+1; j < tokens.size(); j++) {
                        std::cout << " " << id_to_name(tokens[j]->get_name()) << " ";
                    }
                    std::cout << std::endl;
                    std::cout << "Groups new size: " << groups.size() << std::endl;*/
                }
                continue;
            }
            if(type == IDENTIFIER && i+1 < tokens.size()) {
                if(tokens[i+1]->get_name() == LEFT_PAREN) {
                    auto* proc = resolve_proc(tk->get_lexeme());

                    if(proc) {
                        std::vector<std::vector<std::shared_ptr<token>>> arguments;
                        auto curr_arg = std::vector<std::shared_ptr<token>>();
                        int j = i+2;
                        int num_left_paren = 1;
                        while (tokens[j]->get_name() != RIGHT_PAREN || num_left_paren != 0) {
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
                            if(j >= tokens.size()) {
                                lang::interpreter::error("Expected ')' in procedure call");
                                return std::make_shared<token_group>();
                            }
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
                            /*std::cout << "START ARGS: " << std::endl;
                            for (const auto& tok : vec) {
                                std::cout << "ARG: " << *tok << std::endl;
                            }
                            std::cout << "END ARGS: " << std::endl;*/

                        }
                        //std::cout << "PROC NAME: " << tk->get_lexeme() << std::endl;
                        auto proc_ptr = std::make_shared<token>(PROC, tk->get_lexeme(), 0, grouped_args);
                        tokens[i] = proc_ptr;
                        auto wrapper = std::make_shared<token_group>();
                        std::shared_ptr<token_group::token_element> element = std::make_shared<token_group::token_element>(proc_ptr);

                        wrapper->tokens.push_back(element);
                        groups.back()->tokens.push_back(std::make_shared<token_group::token_element>(wrapper));
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

                groups.back()->add(element); // Add this new group to the current group
                groups.push_back(new_group); // Make this the current group
            } else if (type == RIGHT_PAREN) {
                // End the current group and go back to the parent group
                groups.pop_back(); // Pop without adding, it's already added when LEFT_PAREN was encountered
                if (groups.empty()) {
                    // Error handling: encountered a closing parenthesis without a matching opening one
                    return std::make_shared<token_group>(); // Return an empty group or handle error
                }
            }
            else {
                // Add the token to the current group
                std::shared_ptr<token_group::token_element> element = std::make_shared<token_group::token_element>(tk);
                groups.back()->add(element);
            }
        }

        if (groups.size() != 1) {
            // Error handling: not all groups were properly closed
            return std::make_shared<token_group>(); // Return an empty group or handle error
        }

        // At this point, `root_group` contains all tokens properly grouped
        return root_group; // Return the root group
    }

std::shared_ptr<token_group> token_grouper::gen_group(std::vector<std::shared_ptr<token>> tokens) {
    generate_parens(tokens);
    auto g = recursive_group(tokens);
    return g;
}
