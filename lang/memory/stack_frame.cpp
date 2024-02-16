#include "stack_frame.h"

#include "../interpreter.h"
#include "proc_manager.h"
bool stack_frame::set(const char *identifier, data *val) {
    // if it already exists, delete old value
    if(exists(identifier)) {
        delete_var(identifier);
    }
    return memory->set(identifier, val);
}

bool stack_frame::exists(const char *identifier) {
    return memory->exists(identifier);
}

bool stack_frame::assign(const char *identifier,const char *identifier2) {
    data* val = memory->get(identifier2);
    if(!val) {
        lang::interpreter::error("Undefined variable with name: " + std::string(identifier2));
        return false;
    }
    return memory->assign(identifier, val);
}

void stack_frame::delete_var(const char *identifier) {
    memory->delete_var(identifier);
}

data * stack_frame::get_data(const char *identifier) {
    data* result = memory->get(identifier);
    return result;
}

void stack_frame::insert_proc(const std::string &name, int type, proc_tokens* p, proc_type_vec* v) {
    procs->insert_proc(name,type,p,v);
}

proc* stack_frame::resolve_proc(const std::string& name) {
    return procs->resolve_proc_name(name);
}

void stack_frame::eval_proc(std::shared_ptr<token_group> &g) {
    procs->execute_proc(g);
}
