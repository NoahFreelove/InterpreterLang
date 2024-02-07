#ifndef TOKEN_GROUPER_H
#define TOKEN_GROUPER_H
#include <stack>

#include "token.h"
#include "token_group.h"
class token_grouper {
public:
    static token_group* recursive_group(std::vector<token*> tokens) {
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
        std::stack<token_group*> groups;
        token_group* root_group = new token_group(); // This will be the root group
        groups.push(root_group); // Start with the root group on the stack

        for (auto* tk : tokens) {
            int type = tk->get_name();

            if (type == LEFT_PAREN) {
                // Start a new group for everything inside the parentheses
                token_group* new_group = new token_group();
                groups.top()->add(new_group); // Add this new group to the current group
                groups.push(new_group); // Make this the current group
            } else if (type == RIGHT_PAREN) {
                // End the current group and go back to the parent group
                groups.pop(); // Pop without adding, it's already added when LEFT_PAREN was encountered
                if (groups.empty()) {
                    // Error handling: encountered a closing parenthesis without a matching opening one
                    delete root_group;
                    return new token_group(); // Return an empty group or handle error
                }
            } else {
                // Add the token to the current group
                groups.top()->add(tk);
            }
        }

        if (groups.size() != 1) {
            // Error handling: not all groups were properly closed
            delete root_group;
            return new token_group(); // Return an empty group or handle error
        }

        // At this point, `root_group` contains all tokens properly grouped
        return root_group; // Return the root group
    }
};
#endif //TOKEN_GROUPER_H
