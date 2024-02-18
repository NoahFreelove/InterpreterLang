#ifndef CONTROL_FLOW_RUNNER_H
#define CONTROL_FLOW_RUNNER_H
class control_flow_runner {
public:
    static bool process_if(const std::vector<std::shared_ptr<token>>& tokens) {
        std::shared_ptr<token_group> group = lang::interpreter::evaluate_tokens(tokens, 1);
        if(group->type == TRUE || group->type == FALSE) {
            lang::interpreter::if_block_statuses->push(group->type == TRUE);
            if(!lang::interpreter::proc_num_ifs->empty()) {
                int num = lang::interpreter::proc_num_ifs->top() + 1;
                lang::interpreter::proc_num_ifs->pop();
                lang::interpreter::proc_num_ifs->push(num);
            }
            std::cout << "If result: " << lang::interpreter::if_block_statuses->top() << std::endl;
            return group->type == TRUE;
        }
        else {
            lang::interpreter::error("cannot use non-truthy type with if statement");
            return false;
        }
    }
    static void process_control_flow(const std::vector<std::shared_ptr<token>>& tokens) {
        // if its a new if block we reset this
        if(tokens[0]->get_name() == IF) {
            if(process_if(tokens)) {
                //lang::interpreter::if_results->push(true);
            }
        }
        else if (tokens[0]->get_name() == ELSE_IF) {
            if(process_if(tokens)) {
                //lang::interpreter::if_results->push(true);
            }
        }
        else if(tokens[0]->get_name() == ELSE) {
            //lang::interpreter::if_results->push(true);
        }
    }
};
#endif //CONTROL_FLOW_RUNNER_H
