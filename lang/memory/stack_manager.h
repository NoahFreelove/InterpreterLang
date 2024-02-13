#ifndef VAR_RESOLVER_H
#define VAR_RESOLVER_H
#include "data.h"
static data* resolve_variable(const char* identifier){
    data* found = nullptr;
    for (stack_frame* frame : *lang::interpreter::stack) {
        auto result = frame->get_data(identifier);
        if(result) {
            found = result;
        }
    }

    return found;
}

static bool does_variable_exist(const char* identifier) {
    for (stack_frame* frame : *lang::interpreter::stack) {
        if(frame->exists(identifier))
            return true;
    }
    return false;
}

static void assign_variable(const char* identifier1, const  char* identifier2) {
    for (stack_frame* frame : *lang::interpreter::stack) {
        if(frame->assign(identifier1,identifier2))
            break;
    }
}

static void delete_variable(const char* identitifer) {
    for (stack_frame* frame : *lang::interpreter::stack) {
        frame->delete_var(identitifer);
    }
}

static void pop_stackframe() {
    stack_frame* frame = lang::interpreter::stack->back();
    if(frame->get_id() == 1) {
        lang::interpreter::error("Attempt to pop global stack frame");
    }
    else {
        lang::interpreter::stack->pop_back();
        delete frame;
    }
}

static void push_stackframe() {
    auto* new_frame = new stack_frame();
    new_frame->set("stack_frame", new data(new long(new_frame->get_id()), "long"));
    lang::interpreter::stack->push_back(new_frame);
}
#endif //VAR_RESOLVER_H
