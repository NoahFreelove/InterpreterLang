#include "proc_manager.h"
#include "../interpreter.h"
#include "../memory/stack_manager.h"
#include "../evaluator/group_evaluator.h"
#include "../tokenizer/token_grouper.h"

proc * proc_manager::resolve_proc_name(const std::string &name) {
    // return nullptr if not in procs
    if(procs->find(name) == procs->end()) {
        return nullptr;
    }
    return (*procs)[name];
}

void proc_manager::insert_proc(const std::string &name, proc_tokens *p, proc_type_vec *v) {
    if(procs->find(name) != procs->end()) {
        lang::interpreter::error("Attempted redefinition of proc");
        return;
    }
    procs->insert(std::make_pair(name, new std::pair(p,v)));
}

void push_to_stack_frame(const std::shared_ptr<token_group> &evaled_group, stack_frame* frame, const char* name) {
    switch (evaled_group->type) {
        case UNDETERMINED: {
            lang::interpreter::error("Procedure tried to use undertermined group as variable input");
            break;
        }
        case ERROR: {
            lang::interpreter::error("Procedure tried to use errornous group as variable input");
            break;
        }
        case INT: {
            data* var = new data(new int(std::any_cast<int>(evaled_group->value)), "int", false, false);
            frame->set(name, var);
            break;
        }
        case FLOAT: {
            data* var = new data(new float(std::any_cast<float>(evaled_group->value)), "float", false, false);
            frame->set(name, var);
            break;
        }
        case DOUBLE: {
            data* var = new data(new double(std::any_cast<double>(evaled_group->value)), "double", false, false);
            frame->set(name, var);
            break;
        }
        case LONG: {
            data* var = new data(new long(std::any_cast<long>(evaled_group->value)), "long", false, false);
            frame->set(name, var);
            break;
        }
        case STRING: {
            data* var = new data(new std::string(std::any_cast<std::string>(evaled_group->value)), "string", false, false);
            frame->set(name, var);
            break;
        }
        case TRUE:
            case FALSE:{
            data* var = new data(new bool(evaled_group->type == TRUE), "bool", false, false);
            frame->set(name, var);
            break;
        }


        default: {
            lang::interpreter::error("Procedure tried to use unknown group type as variable input");
        }
    }
}

void push_byref(const char* var_name, stack_frame* frame, const char* new_name) {
    auto* var = resolve_variable(var_name);
    if(var) {
        if(!frame->set(new_name, var)) {
            lang::interpreter::error("Failed to pass var by ref in procedure");
        }
    }else {
        lang::interpreter::error("Attempted to pass variable by reference into procedure "
                                 "which deos not exist in the current context!");
    }
}
void push_val_directly(const std::shared_ptr<token> &t, stack_frame* frame, const char* name) {
    switch (t->get_name()) {
        case INT: {
            data* var = new data(new int(std::any_cast<int>(t->get_value())), "int", false, false);
            frame->set(name, var);
            break;
        }
        case FLOAT: {
            data* var = new data(new float(std::any_cast<float>(t->get_value())), "float", false, false);
            frame->set(name, var);
            break;
        }
        case DOUBLE: {
            data* var = new data(new double(std::any_cast<double>(t->get_value())), "double", false, false);
            frame->set(name, var);
            break;
        }
        case LONG: {
            data* var = new data(new long(std::any_cast<long>(t->get_value())), "long", false, false);
            frame->set(name, var);
            break;
        }
        case STRING: {
            data* var = new data(new std::string(std::any_cast<std::string>(t->get_value())), "string", false, false);
            frame->set(name, var);
            break;
        }
        case TRUE:
            case FALSE:{
            data* var = new data(new bool(t->get_name() == TRUE), "bool", false, false);
            frame->set(name, var);
            break;
        }
        default: {
            lang::interpreter::error("Procedure tried to use unknown token type as variable input");
        }
    }
}

void proc_manager::execute_proc(std::shared_ptr<token_group> &g) {
    //std::vector<std::shared_ptr<token_group>> args = g.
    auto* new_frame = new stack_frame();
    g->type = NOTHING; // Return type
    g->value = nullptr; // Return value

    if(group_evaluator::is_group(*g->tokens[0]) ) {
        lang::interpreter::error("Cannot run proc on group?");
        return;
    }
    auto tok = (*std::get<std::shared_ptr<token>>(*g->tokens[0]));
    proc* p = resolve_proc(tok.get_lexeme());

    proc_type_vec types = *p->second;


    auto args = std::any_cast<std::vector<std::shared_ptr<token_group>>>(tok.get_value());
    if(args.size() != types.size()) {
        lang::interpreter::error("Number of args don't match procedure definition");
        return;
    }

    int i = 0;
    for (const auto& arg : args) {
        //std::cout << "arg found. num tokens: " << arg->tokens.size() << std::endl;

        if(arg->tokens.size() == 1) {
            if(!group_evaluator::is_group(*arg->tokens[0])) {
                token t = *std::get<std::shared_ptr<token>>(*arg->tokens[0]);
                if(t.get_name() == IDENTIFIER) {
                    // dupe variable, pass byref
                    push_byref(t.get_lexeme(), new_frame,types[i].second->get_lexeme());
                }
                else {
                    push_val_directly(std::get<std::shared_ptr<token>>(*arg->tokens[0]), new_frame, types[i].second->get_lexeme());
                }
            }
            else {
                auto group_ref = std::get<std::shared_ptr<token_group>>(*arg->tokens[0]);
                group_evaluator::eval_group(group_ref);
                push_to_stack_frame(group_ref, new_frame, types[i].second->get_lexeme());
            }
        }
        else {
            group_evaluator::eval_group(arg);
            push_to_stack_frame(arg, new_frame, types[i].second->get_lexeme());
        }
        /*std::cout << "is group: " << group_evaluator::is_group(*arg->tokens[0]) << std::endl;
        std::cout << "type should be: " << id_to_name(types[i].first->get_name()) << std::endl;*/

        i++;
    }

    push_stackframe(new_frame);

    // resolve proc checks all stacks, not just current one
    for (const auto& t : *p->first) {
        lang::interpreter::queue.push(t);
    }
    lang::interpreter::queue.push({std::make_shared<token>(RIGHT_BRACE, "}",0)});
    lang::interpreter::run();
}

void proc_manager::print_procs() {
    for (const auto& elm : *procs) {
        std::cout << elm.first << "(";
        int i = 0;
        for (const auto& t : (*elm.second->second)) {
            std::cout << id_to_name(t.first->get_name()) << " " << t.second->get_lexeme();
            i++;
            if(i < elm.second->second->size()) {
                std::cout << ", ";
            }
            else {
                std::cout << ')' << std::endl;
            }
        }
    }
    if(procs->empty())
        std::cout << "none..." << std::endl;
}