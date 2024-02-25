#ifndef LOOP_EXECUTOR_H
#define LOOP_EXECUTOR_H
#include "../memory/proc_manager.h"
#include "var_setter.h"
typedef proc_tokens loop_data;
struct loop{
    int type = 0;
    // index isnt the loop index but the index that this loop is in the nested loop hierarchy
    int index = -1;

    int loop_ifs = 0;

    bool is_do = false;

    bool is_until = false;

    bool is_generic_loop = false;

    loop_data* loop_lines = nullptr;

    // while and for
    std::shared_ptr<token_group> condition_test;

    // for exclusive
    std::shared_ptr<token> iterator_variable = nullptr;
    std::vector<std::shared_ptr<token>> post_for_event;

    // foreach
    data* array_ptr;
    std::string foreach_var_name;
    data* index_var;
    int foreach_size = 0;
    int foreach_index;

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
    inline static int loop_declaration_nest = 0;
    inline static bool trigger_break = false;
    inline static int current_loop_index = -1;

    static bool process_while(loop* l, const lang::interpreter::token_vec & tokens) {
        l->type = WHILE;
        if(tokens[0]->get_name() == DO_WHILE || tokens[0]->get_name() == DO_UNTIl) {
            l->is_do = true;
        }
        if(tokens[0]->get_name() == DO_UNTIl || tokens[0]->get_name() == UNTIL) {
            l->is_until = true;
        }
        if(tokens[0]->get_name() == LOOP)
            l->is_generic_loop = true;
        auto vec_cpy = lang::interpreter::clone_tokens(tokens);
        vec_cpy.erase(vec_cpy.begin());

        auto group = token_grouper::gen_group(vec_cpy);
        if(group->tokens.size() != 1 && tokens[0]->get_name() != LOOP) {
            lang::interpreter::error("Invalid loop condition, must be one group");
            return false;
        }
        l->condition_test = group;
        return true;
    }

    static bool process_for(loop* l, const lang::interpreter::token_vec & tokens) {
        l->type = FOR;
        auto vec_cpy = lang::interpreter::clone_tokens(tokens);
        vec_cpy.erase(vec_cpy.begin());
        if(vec_cpy.size() == 0) {
            lang::interpreter::error("Cannot create a for loop with just 'for'!");
            return false;
        }
        std::vector<std::shared_ptr<token>> var_part;
        std::vector<std::shared_ptr<token>> condition_part;
        std::vector<std::shared_ptr<token>> post_loop_part;

        // each part is comma seperated: for int i = 0, i<5, i++
        // parenthesis are required
        if(vec_cpy[0]->get_name() != LEFT_PAREN || vec_cpy[vec_cpy.size()-1]->get_name() != RIGHT_PAREN) {
            lang::interpreter::error("For loop iterator must be contained in parenthesis");
            return false;
        }
        // erase first and last paren
        vec_cpy.erase(vec_cpy.begin());
        vec_cpy.pop_back();

        // var part can be like, int i = 0, or i = 0, or just i, so we really just need to
        // evaluate it in isolation and extract the first identifier's name and clone that token.
        // the condition part
        int comma_count = 0;
        for (auto& tok : vec_cpy) {
            if(tok->get_name() == COMMA) {
                comma_count+=1;
                if(comma_count > 2) {
                    lang::interpreter::error("For loop can only have 3 premises");
                    return false;
                }
                continue;
            }
            if(comma_count == 0) {
                var_part.push_back(tok);
            }
            else if(comma_count == 1) {
                condition_part.push_back(tok);
            }
            else if(comma_count == 2) {
                post_loop_part.push_back(tok);
            }
            else {
                lang::interpreter::error("Invalid amount of commas");
                return false;
            }
        }


        if(!var_part.empty()) {
            if(var_part.size() >= 2) {
                if(var_part[0]->is_typeword() && var_part[1]->is_identifier()) {
                    l->iterator_variable = var_part[1];
                    process_variable_declaration(var_part);
                    if(lang::interpreter::errors->size() > 0) {
                        lang::interpreter::error("Stopped for loop creation. error in first premise");
                        return false;
                    }
                }
                else if(var_part[0]->is_identifier()) {
                    l->iterator_variable = var_part[0];
                    process_variable_update(var_part);
                    if(lang::interpreter::errors->size() > 0) {
                        lang::interpreter::error("Stopped for loop creation. error in first premise");
                        return false;
                    }
                }
            }
            // var_part size must be 1. weird to do a for(i, i<x, i++) though...
            else if(var_part[0]->is_identifier()) {
                l->iterator_variable = var_part[0];
            }
        }
        else {
            l->iterator_variable = nullptr;
        }

        if(condition_part.empty()) { // This is for(int i,,i++), basically a while (true) loop, but not illegal input
            std::vector<std::shared_ptr<token>> vec;
            vec.push_back(std::make_shared<token>(TRUE, "TRUE",0, true));
            l->condition_test = token_grouper::gen_group(vec);
        }
        else {
            l->condition_test = token_grouper::gen_group(lang::interpreter::clone_tokens(condition_part));
        }

        l->post_for_event = lang::interpreter::clone_tokens(post_loop_part);
        return true;
    }

    static bool process_foreach(loop * l, const lang::interpreter::token_vec & tokens) {
        l->type = FOREACH;
        auto vec_cpy = lang::interpreter::clone_tokens(tokens);
        vec_cpy.erase(vec_cpy.begin());
        if(vec_cpy.empty()) {
            lang::interpreter::error("Cannot create a foreach loop with just 'for'!");
            return false;
        }
        std::vector<std::shared_ptr<token>> var_part;
        std::string array_name;
        std::string index_var_name;

        // each part is comma seperated: for int i = 0, i<5, i++
        // parenthesis are required
        if(vec_cpy[0]->get_name() != LEFT_PAREN || vec_cpy[vec_cpy.size()-1]->get_name() != RIGHT_PAREN) {
            lang::interpreter::error("For loop iterator must be contained in parenthesis");
            return false;
        }
        // erase first and last paren
        vec_cpy.erase(vec_cpy.begin());
        vec_cpy.pop_back();

        int typeword;
        if(!vec_cpy[0]->is_typeword()) {
            lang::interpreter::error("Foreach loop must have a type word");
            return false;
        }
        else {
            typeword = vec_cpy[0]->get_name();
        }
        vec_cpy.erase(vec_cpy.begin());

        std::string var_name;
        if(vec_cpy[0]->is_identifier()) {
            var_name = std::string(vec_cpy[0]->get_lexeme());
        }
        else {
            lang::interpreter::error("Foreach loop must have a variable name");
            return false;
        }
        vec_cpy.erase(vec_cpy.begin());

        if(vec_cpy[0]->get_name() == COMMA && vec_cpy.size()>1) {
            if(vec_cpy[1]->get_name() == IDENTIFIER) {
                index_var_name = std::string(vec_cpy[1]->get_lexeme());
                vec_cpy.erase(vec_cpy.begin(), vec_cpy.begin()+2);
            }
            else {
                lang::interpreter::error("Expected index variable name after comma");
                return false;
            }
        }

        if(vec_cpy[0]->get_name() != COLON) {
            lang::interpreter::error("Foreach loop must have a colon to denote start of array");
            return false;
        }
        vec_cpy.erase(vec_cpy.begin());

        if(vec_cpy[0]->get_name() != IDENTIFIER) {
            lang::interpreter::error("Foreach loop must have an array name");
            return false;
        }
        array_name = std::string(vec_cpy[0]->get_lexeme());
        vec_cpy.erase(vec_cpy.begin());
        if(!vec_cpy.empty()) {
            lang::interpreter::error("Unexpected tokens in foreach loop declaration");
            return false;
        }


        data* arr = resolve_variable(array_name.c_str());
        if(!arr) {
            lang::interpreter::error("Undefined array in foreach loop");
            return false;
        }
        else if(!arr->is_array()) {
            lang::interpreter::error("Foreach loop must have an array");
            return false;
        }
        if(token::typeword_to_type(typeword) != arr->get_type_int()) {

            lang::interpreter::error("Type mismatch in foreach loop");
            return false;
        }
        l->array_ptr = arr;
        l->foreach_var_name = var_name;
        l->foreach_size = arr->get_arr_size();
        if(index_var_name != "") {
            data* index_var = new data(new int(0),"int");
            l->index_var = index_var;
            lang::interpreter::top_stack()->set(index_var_name, index_var);
        }

        if(l->foreach_size == -1) {
            lang::interpreter::error("Cannot determine size of array in foreach loop");
            return false;
        }
        return true;
    }

    static void process_loop(const lang::interpreter::token_vec & tokens) {
        loop* l = new loop();
        if(tokens[0]->is_while_variation()) {
            if(!process_while(l, tokens)) {
                delete l;
                return;
            }
        }
        else if(tokens[0]->get_name() == FOR) {
            if(!process_for(l, tokens)) {
                delete l;
                return;
            }
        }
        else if (tokens[0]->get_name() == FOREACH) {
            if(!process_foreach(l,tokens)) {
                delete l;
                return;
            }
        }
        else {
            lang::interpreter::error("Invalid loop type");
            delete l;
            return;
        }

        in_loop_declaration = true;
        current_loop = l;
        loop_declaration_nest = 1;
    }

    static bool is_condition_true(loop* l) {
        if(l->is_generic_loop)
            return true;
        auto group = token_grouper::recursive_clone_group(l->condition_test);
        group_evaluator::eval_group(group);
        int type = group->type;
        if(type == TRUE || type == FALSE) {
            //std::cout << "result of condition: " << (type == TRUE) << std::endl;
            if(l->is_until) {
                return type == FALSE;
            }
            return type == TRUE;
        }
        lang::interpreter::error("Loop condition does not evaluate to a truthy value!");
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

    static void end_loop(loop* l) {
        if(trigger_break)
            trigger_break = false;
        // remove all loops after l->index including this one
        for(int i = l->index; i < active_loops.size(); i++) {
            delete active_loops[i];
        }
        active_loops.erase(active_loops.begin() + l->index, active_loops.end());
    }

    static void trigger_for_loop(loop* l) {
        bool condition_result = is_condition_true(l);
        while (condition_result && !trigger_break) {
            current_loop_index = l->index;
            auto q = clone_loop(l);
            q.push(lang::interpreter::clone_tokens(l->post_for_event));

            lang::interpreter::queue_lines(q, LOOP_INPUT);
            lang::interpreter::trigger_run();

            condition_result = is_condition_true(l);
            lang::interpreter::print_errs();
        }
        end_loop(l);
    }

    static void trigger_foreach_loop(loop* l) {
        int len = l->foreach_size;
        for (int i = 0; i < len; i++) {
            current_loop_index = l->index;
            // update iterator variable
            data* newdat = new data(l->array_ptr->get_array_element(i), true);
            lang::interpreter::top_stack()->set(l->foreach_var_name, newdat);
            l->foreach_index = i;
            if(l->index_var) {
                l->index_var->set_value_int(i);
            }

            auto q = clone_loop(l);
            q.push(lang::interpreter::clone_tokens(l->post_for_event));

            lang::interpreter::queue_lines(q, LOOP_INPUT);
            lang::interpreter::trigger_run();

            lang::interpreter::print_errs();
        }
        end_loop(l);
    }

    static void trigger_while_loop(loop* l) {
        bool condition_result;
        if(l->is_do)
            condition_result = true;
        else
            condition_result = is_condition_true(l);

        // TODO: Optimize this so lines which can be pre-evaluated are
        // so that this is less intensive.
        while (condition_result && !trigger_break) {
            current_loop_index = l->index;

            lang::interpreter::queue_lines(clone_loop(l), LOOP_INPUT);
            lang::interpreter::trigger_run();
            condition_result = is_condition_true(l);
            lang::interpreter::print_errs();
        }
        end_loop(l);
    }

    inline static void end_loop_declaration() {
        in_loop_declaration = false;
        current_loop->index = active_loops.size();
        active_loops.push_back(current_loop);
        if(current_loop->type == WHILE) {
            trigger_while_loop(current_loop);
        }
        else if(current_loop->type == FOR) {
            trigger_for_loop(current_loop);
        }
        else if(current_loop->type == FOREACH) {
            trigger_foreach_loop(current_loop);
        }
    }

    static bool in_loop() {
        return !active_loops.empty();
    }
};

#endif //LOOP_EXECUTOR_H
