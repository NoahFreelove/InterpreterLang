#include "proc_manager.h"
#include "../interpreter.h"
#include "../memory/stack_manager.h"
#include "../evaluator/group_evaluator.h"
#include "../tokenizer/token_grouper.h"
#include "../executors/control_flow_runner.h"

proc_dat * proc_manager::resolve_proc_name(const std::string &name) {
    // return nullptr if not in procs
    if(procs->find(name) == procs->end()) {
        return nullptr;
    }
    return (*procs)[name];
}

void proc_manager::insert_proc(const std::string &name, int type, proc_tokens *p, proc_type_vec *v) {
    if(procs->find(name) != procs->end()) {
        if(!lang::interpreter::is_defined("PROC_OVERLOAD")) {
            lang::interpreter::error("Overloading is not enabled, cannot define procedure with same name");
            return;
        }
        bool is_identical = true;
        proc_type_vec* other_types = (*procs)[name]->first->second;
        if(other_types->size() != v->size()) {
            is_identical = false;
        }
        else {
            for (int i = 0; i < v->size(); i++) {
                if((*v)[i].first->get_name() != (*other_types)[i].first->get_name()) {
                    is_identical = false;
                    break;
                }
            }
        }

        if(!lang::interpreter::is_defined("PROC_REDEFINITION") && is_identical) {
            lang::interpreter::error("Redefinition of procedure " + name + " is not enabled");
            return;
        }
        // replace the old proc
        // Change this logic later to handle overloading because it just replaces the old proc
        // TODO: This
        auto* new_pair = new std::pair<proc_tokens*, proc_type_vec*>(p,v);
        (*procs)[name] = new std::pair<std::pair<proc_tokens*, proc_type_vec*>*,int>(new_pair,type);
        return;
    }
    procs->insert(std::make_pair(name, new std::pair(new std::pair(p,v),type)));
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
        case TRUE: {
            data* var = new data(new bool(true), "bool", false, false);
            frame->set(name, var);
            break;
        }
        case FALSE: {
            data* var = new data(new bool(false), "bool", false, false);
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
        {
            data* var = new data(new bool(true), "bool", false, false);
            frame->set(name, var);
            break;
        }
        case FALSE: {
            data* var = new data(new bool(false), "bool", false, false);
            frame->set(name, var);
            break;
        }

        default: {
            lang::interpreter::error("Procedure tried to use unknown token type as variable input");
        }
    }
}

data* push_return_variable(stack_frame* frame, int type) {
    data* return_var = nullptr;
    switch (type) {
        case INT: {
            return_var = new data(new int(0), "int");
            break;
        }
        case LONG: {
            return_var = new data(new long(0), "long");
            break;
        }
        case FLOAT: {
            return_var = new data(new float(0), "float");
            break;
        }
        case DOUBLE: {
            return_var = new data(new double(0), "double");
            break;
        }
        case STRING: {
            return_var = new data(new std::string(), "string");
            break;
        }
        case FALSE: {
            return_var = new data(new bool(false), "bool");
            break;
        }
        case TRUE: {
            return_var = new data(new bool(false), "bool");
            break;
        }
        case BOOL_KEYW: {
            return_var = new data(new bool(false), "bool");
            break;
        }

        case NOTHING_TYPE: {
            return_var = new data(nullptr, "nothing");
            break;
        }
        default: {
            lang::interpreter::error("Unknown return type in procedure");
            return nullptr;
        }
    }
    frame->set("return", return_var);
    return return_var;
}

void proc_manager::execute_proc(std::shared_ptr<token_group> &g) {
    //std::vector<std::shared_ptr<token_group>> args = g.

    auto* new_frame = new stack_frame();

    if(group_evaluator::is_group(*g->tokens[0]) ) {
        lang::interpreter::error("Cannot run proc on group?");
        return;
    }
    auto tok = (*std::get<std::shared_ptr<token>>(*g->tokens[0]));
    proc_dat* p = resolve_proc(tok.get_lexeme());
    int PROC_TYPE = p->second;
    auto* dat = push_return_variable(new_frame, PROC_TYPE);
    g->type = p->second; // Return type
    if(g->type == NOTHING_TYPE) {
        g->type = NOTHING;
    }
    g->value = nullptr; // Return value

    proc_type_vec types = *p->first->second;

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
    std::queue<std::vector<std::shared_ptr<token>>> proc_toks;

    for (const auto& t : *p->first->first) {
        proc_toks.push(t);
    }
    proc_toks.push({std::make_shared<token>(RETURN, "return",0)});
    lang::interpreter::proc_num_ifs->push(0);
    lang::interpreter::num_procs_active++;
    lang::interpreter::queue_stack.push(proc_toks);
    lang::interpreter::run();

    std::queue<std::vector<std::shared_ptr<token>>> right_brace;
    // in the event they forgot.
    right_brace.push({std::make_shared<token>(RIGHT_BRACE, "}",0)});

    if(dat) {
        // Convert dat return from ptr to value
        switch (g->type) {
            case INT: {
                g->value = dat->get_int();
                break;
            }
            case LONG: {
                g->value = dat->get_long();
                break;
            }
            case FLOAT: {
                g->value = dat->get_float();
                break;
            }
            case DOUBLE: {
                g->value = dat->get_double();
                break;
            }
            case STRING: {
                g->value = dat->get_string();
                break;
            }
            case TRUE: {
                g->value = dat->get_bool();
                break;
            }
            case FALSE: {
                g->value = dat->get_bool();
                break;
            }
            case BOOL_KEYW: {
                g->type = (dat->get_bool() ? TRUE : FALSE);
                g->value = dat->get_bool();
                break;
            }
            case NOTHING: {
                g->value = nullptr;
                break;
            }
            default: {
                lang::interpreter::error("Unknown return type in procedure close");
            }
        }
    }
    else {
        std::cout <<"Invalid data" << std::endl;
    }


    lang::interpreter::queue_stack.push(right_brace);
    lang::interpreter::run();
}

void proc_manager::print_procs() {
    for (const auto& elm : *procs) {
        std::cout << elm.first << "(";
        int i = 0;
        for (const auto& t : (*elm.second->first->second)) {
            std::cout << id_to_name(t.first->get_name()) << " " << t.second->get_lexeme();
            i++;
            if(i < elm.second->first->second->size()) {
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

void proc_manager::process_return(const lang::interpreter::token_vec &tokens, int offset) {
    //std::cout << "Processing return" << std::endl;
    data* dat = lang::interpreter::stack->back()->get_data("return");
    if(!dat) {
        lang::interpreter::error("Not currently in a proc with a return variable, cannot return value!");
        return;
    }
    if(lang::interpreter::proc_num_ifs->empty()) {
        lang::interpreter::error("Proc was not properly instantiated, cannot check if blocks used");
    }
    else {
        int if_count = lang::interpreter::proc_num_ifs->top();
        lang::interpreter::proc_num_ifs->pop();

        for (int i = 0; i < if_count; ++i) {
            if(control_flow_runner::blockStack.empty()) {
                //lang::interpreter::error("Could not pop if blocks - proc num ifs too big");
                break;
            }
            else {
                control_flow_runner::blockStack.pop();
            }
        }
    }
    bool implicit_upcast = lang::interpreter::is_defined("IMPLICIT_UPCAST");
    bool implicit_double_float = lang::interpreter::is_defined("IMPLICIT_DOUBLE_TO_FLOAT");
    lang::interpreter::token_vec rest = lang::interpreter::token_vec();
    rest.reserve(tokens.size()-offset);
    for (int i = offset; i < tokens.size(); i++) {
        rest.push_back(tokens[i]);
    }
    auto group = token_grouper::gen_group(rest);
    group_evaluator::eval_group(group);

    int target_type = dat->get_type_int();

    if(group->type == ERROR || group->type == UNDETERMINED) {
        lang::interpreter::error("Could not process return value");
        return;
    }
    else if (group->type == NOTHING_TYPE && target_type == NOTHING) {
        dat->set_nullptr();
        return;
    }
    else if(group->type == NOTHING) {
        // the default value for each datatype is already set
        // so like if they forgot a return for type int, 0 would be implicity returned
        // :) I dont like null vars!
        return;
    }
    if(target_type == group->type) {
        switch (target_type) {
            case INT: {
                dat->set_value_int(std::any_cast<int>(group->value));
                return;
            }
            case FLOAT: {
                dat->set_value_float(std::any_cast<float>(group->value));
                return;
            }
            case DOUBLE: {
                dat->set_value_double(std::any_cast<double>(group->value));
                return;
            }
            case LONG: {
                dat->set_value_long(std::any_cast<long>(group->value));
                return;
            }
            case STRING: {
                dat->set_value_string(std::any_cast<std::string>(group->value));
                return;
            }
            case ULONG64: {
                dat->set_value_ulonglong(std::any_cast<unsigned long long>(group->value));
                return;
            }

            default: {
                lang::interpreter::error("Return type not supported");
                break;
            }
        }
    }
    if(target_type == BOOL_KEYW && (group->type == TRUE || group->type == FALSE)) {
        dat->set_value_bool(group->type == TRUE);
        return;
    }
    if(target_type == FLOAT && group->type == DOUBLE && implicit_double_float) {
        dat->set_value_float(std::any_cast<double>(group->value));
        return;
    }
    if(!implicit_upcast) {
        lang::interpreter::error("Return type does not match procedure type (" + id_to_name(target_type) + " !=" + id_to_name(group->type) + ")");
        return;
    }
    else if(target_type == FLOAT && group->type == INT) {
        dat->set_value_float(std::any_cast<int>(group->value));
    }
    else if(target_type == FLOAT && group->type == LONG) {
        dat->set_value_float(std::any_cast<long>(group->value));
    }
    else if(target_type == DOUBLE && group->type == INT) {
        dat->set_value_double(std::any_cast<int>(group->value));
    }
    else if(target_type == DOUBLE && group->type == LONG) {
        dat->set_value_double(std::any_cast<long>(group->value));
    }
    else if(target_type == DOUBLE && group->type == FLOAT) {
        dat->set_value_double(std::any_cast<float>(group->value));
    }
    else if(target_type == LONG && group->type == INT) {
        dat->set_value_long(std::any_cast<int>(group->value));
    }
    else {
        lang::interpreter::error("Return upcast type not supported. Cannot upcast " + id_to_name(group->type) + " to " + id_to_name(target_type));
    }

}