#include "stack_frame.h"

#include "../interpreter.h"

bool stack_frame::set(char *identifier, data *val) {
    return memory->set(identifier, val);
}

bool stack_frame::assign(char *identifier, char *identifier2) {
    data* val = memory->get(identifier2);
    if(!val) {
        lang::interpreter::error("Undefined variable with name: " + std::string(identifier2));
        return false;
    }
    return memory->assign(identifier, val);
}

void stack_frame::delete_var(char *identifier) {
    memory->delete_var(identifier);
}

data * stack_frame::get_data(char *identifier) {
    data* result = memory->get(identifier);
    if (result == nullptr) {
        lang::interpreter::error("Undefined variable with name: " + std::string(identifier));
        return nullptr;
    }
    return result;
}
