#ifndef LOOP_EXECUTOR_H
#define LOOP_EXECUTOR_H
#include "../memory/proc_manager.h"
typedef proc_tokens loop_data;
struct loop{
    int type = 0;
    int index = -1;

    int loop_ifs = 0;

    loop_data* loop_lines = nullptr;
    std::vector<std::shared_ptr<token>> condition;
    std::shared_ptr<token_group> condition_test;
    std::shared_ptr<token> iterator_variable = nullptr;

    loop() {
        loop_lines = new loop_data();
    }

    ~loop() {
        delete loop_lines;
    }
};

class loop_executor {
public:
    inline static std::vector<loop*> active_loops = std::vector<loop*>();
    inline static loop* current_loop = nullptr;
    inline static bool in_loop_declaration = false;
    inline static bool trigger_break = false;
    inline static int current_loop_index = -1;

    inline static void process_loop(const lang::interpreter::token_vec & tokens) {
        loop* l = new loop();
        if(tokens[0]->get_name() == WHILE) {
            l->type = WHILE;
            auto vec_cpy = lang::interpreter::clone_tokens(tokens);
            vec_cpy.erase(vec_cpy.begin());

            auto group = token_grouper::gen_group(vec_cpy);
            if(group->tokens.size() != 1) {
                delete l;
                lang::interpreter::error("Invalid loop condition, must be one group");
                return;
            }
            l->condition_test = group;
            l->condition = vec_cpy;
        }
        else if(tokens[0]->get_name() == FOR) {
            l->type = FOR;
            lang::interpreter::error("For loops are not yet implemented");
            // TODO: DELETE THIS WHEN FOR LOOPS ARE IMPLEMENTED :D
            delete l;
            return;
        }
        else {
            lang::interpreter::error("Invalid loop type");
            delete l;
            return;
        }

        in_loop_declaration = true;
        current_loop = l;
    }



    static bool is_condition_true(loop* l) {
        auto group = token_grouper::recursive_clone_group(l->condition_test);
        group_evaluator::eval_group(group);
        int type = group->type;
        if(type == TRUE || type == FALSE) {
            //std::cout << "result of condition: " << (type == TRUE) << std::endl;
            return type == TRUE;
        }
        lang::interpreter::error("While condition does not evaluate to a truthy value!");
        trigger_break = true;
        return false;
    }

    static std::queue<std::vector<std::shared_ptr<token>>> clone_loop(loop* l) {
        std::queue<std::vector<std::shared_ptr<token>>> q;
        for(const std::vector<std::shared_ptr<token>>& t : *(l->loop_lines)) {
            q.push(lang::interpreter::clone_tokens(t));
        }
        return q;
    }

    static void trigger_while_loop(loop* l) {
        bool condition_result = is_condition_true(l);

        // TODO: Optimize this so lines which can be pre-evaluated are
        // so that this is less intensive.
        while (condition_result && !trigger_break) {
            current_loop_index = l->index;

            lang::interpreter::queue_lines(clone_loop(l), LOOP_INPUT);
            lang::interpreter::trigger_run();
            condition_result = is_condition_true(l);
        }
        if(trigger_break)
            trigger_break = false;
        // remove all loops after l->index including this one
        for(int i = l->index; i < active_loops.size(); i++) {
            delete active_loops[i];
        }
        active_loops.erase(active_loops.begin() + l->index, active_loops.end());
    }

    inline static void end_loop_declaration() {
        in_loop_declaration = false;
        current_loop->index = active_loops.size();
        active_loops.push_back(current_loop);
        if(current_loop->type == WHILE) {
            trigger_while_loop(current_loop);
        }
        else if(current_loop->type == FOR) {
            lang::interpreter::error("For loops are not yet implemented");
        }
    }

    static bool in_loop() {
        return !active_loops.empty();
    }
};

#endif //LOOP_EXECUTOR_H
