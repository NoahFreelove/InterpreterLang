#ifndef CONTROL_FLOW_RUNNER_H
#define CONTROL_FLOW_RUNNER_H
#include "loop_executor.h"

// Define a structure for block status
struct BlockStatus {
    bool isExecuting;    // Is the current block being executed?
    bool wasAnyTrue;     // Was any prior block in the chain true (and executed)?
};

class control_flow_runner {
public:
    static inline std::stack<BlockStatus> blockStack = std::stack<BlockStatus>();
    static bool shouldExecuteCurrentBlock() {
        if (blockStack.empty()) {
            // Fallback or error handling; ideally, this should never happen
            // if the stack is managed correctly with a global scope always present.
            return true;
        }
        return blockStack.top().isExecuting;
    }

    static void handleIf(const std::vector<std::shared_ptr<token>>& tokens) {
        bool conditionResult = process_if(tokens);
        blockStack.push({conditionResult, conditionResult});
    }

    static void handleElseIf(const std::vector<std::shared_ptr<token>>& tokens) {
        // Assuming there's always an 'if' or 'else if' before an 'else if'
        BlockStatus& top = blockStack.top();
        if (!top.wasAnyTrue) { // Only check condition if no previous block was true
            bool conditionResult = process_if(tokens);
            top.isExecuting = conditionResult;
            top.wasAnyTrue = top.isExecuting;
        } else {
            top.isExecuting = false; // Don't execute this block since a previous one was true
        }
    }

    static void handleElse() {
        BlockStatus& top = blockStack.top();
        top.isExecuting = !top.wasAnyTrue; // Execute else block only if no previous block was true
        top.wasAnyTrue = true; // Prevent execution of blocks that might come after else (though syntactically incorrect)
    }

    static void handleEndIf() {
        if (!blockStack.empty()) {
            blockStack.pop();
        }
    }
public:
    static bool process_if(const std::vector<std::shared_ptr<token>>& tokens) {
        std::shared_ptr<token_group> group = lang::interpreter::evaluate_tokens(tokens, 1);
        //std::cout << id_to_name(group->type) << std::endl;
        if(group->type == TRUE || group->type == FALSE) {
            //std::cout << "If result: " << (group->type == TRUE) << std::endl;

            if(group->type == TRUE) {
                // TODO: edit this so it supports procs and loops together
                // right now if a loop calls an if it will be double counted
                // potentially add a seperate queue stack for attributes (like an int)
                // if the current stack runs the queue and its a proc
                // run the following, same if its a loop.
                // when the queue is popped the next attribute is checked
                // if no attributes, dont run either of the below
                if(!lang::interpreter::proc_num_ifs->empty()) {
                    int num = lang::interpreter::proc_num_ifs->top() + 1;
                    lang::interpreter::proc_num_ifs->pop();
                    lang::interpreter::proc_num_ifs->push(num);

                }
                else {
                    lang::interpreter::proc_num_ifs->push(1);
                }
                if(loop_executor::current_loop_index != -1) {
                    loop_executor::active_loops[loop_executor::current_loop_index]->loop_ifs++;
                }
                return true;
            }
            return false;
        }
        else {
            lang::interpreter::error("cannot use non-truthy type with if statement");
            return false;
        }
    }
    static void process_control_flow(const std::vector<std::shared_ptr<token>>& tokens) {
        if(tokens.empty()) return; // Safety check

        //std::cout << "Handling: " + id_to_name(tokens[0]->get_name()) << std::endl;

        if(tokens[0]->get_name() == IF) {
            handleIf(tokens);
        }
        else if (tokens[0]->get_name() == ELSE_IF) {
            handleElseIf(tokens);
        }
        else if(tokens[0]->get_name() == ELSE) {
            handleElse();
        }
    }
};
#endif //CONTROL_FLOW_RUNNER_H
