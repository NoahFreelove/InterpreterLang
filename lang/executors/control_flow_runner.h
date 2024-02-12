#ifndef CONTROL_FLOW_RUNNER_H
#define CONTROL_FLOW_RUNNER_H
class control_flow_runner {
public:
    static void process_if(const std::vector<std::shared_ptr<token>>& tokens) {
        std::shared_ptr<token_group> group = lang::interpreter::evaluate_tokens(tokens, 1);
        if(group->type == TRUE || group->type == FALSE) {
            lang::interpreter::if_block_statuses->push(group->type == TRUE);
            std::cout << "If result: " << lang::interpreter::if_block_statuses->top() << std::endl;
        }
        else {
            lang::interpreter::error("cannot use non-truthy type with if statement");
            return;
        }
    }
    static void process_control_flow(const std::vector<std::shared_ptr<token>>& tokens) {
        // if its a new if block we reset this
        if(tokens[0]->get_name() == IF) {
            process_if(tokens);
        }
        else if (tokens[0]->get_name() == ELSE_IF) {
            process_if(tokens);
        }
        else if(tokens[0]->get_name() == ELSE) {

        }
    }
};
#endif //CONTROL_FLOW_RUNNER_H
