#include "stack_frame.h"

#include "../interpreter.h"
#include "proc_manager.h"
bool stack_frame::set(const std::string& identifier, data *val) {
    // if it already exists, delete old value
    if(exists(identifier)) {
        delete_var(identifier);
    }
    return memory->set(identifier, val);
}

bool stack_frame::exists(const std::string& identifier) {
    return memory->exists(identifier);
}

bool stack_frame::assign(const std::string& identifier,const std::string& identifier2) {
    data* val = memory->get(identifier2);
    if(!val) {
        lang::interpreter::error("Undefined variable with name: " + std::string(identifier2));
        return false;
    }
    return memory->assign(identifier, val);
}

bool stack_frame::assign(data* a,data* b) {
    return memory->assign(a, b);
}

void stack_frame::delete_var(const std::string& identifier, bool force) {
    memory->delete_var(identifier, force);
}

data * stack_frame::get_data(const std::string& identifier) {
    data* result = memory->get(identifier);
    if(result) {
        if(result->get_type() == "nothing" && identifier != "return") {
            // remove the nothing value from the stack
            delete_var(identifier);
            return nullptr;
        }
    }
    return result;
}

void stack_frame::insert_proc(const std::string &name, int type, proc_tokens* p, proc_type_vec* v, bool is_native) {
    procs->insert_proc(name,type,p,v, is_native);
}

proc stack_frame::resolve_proc(const std::string& name, proc_type_vec& vec) {
    return procs->resolve_proc_name(name, vec);
}

bool stack_frame::proc_exists(const std::string &name) {
    return procs->exists(name);
}


void stack_frame::eval_proc(std::shared_ptr<token_group> &g) {
    proc_manager::execute_proc(g);
}
