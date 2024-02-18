#ifndef VAR_RESOLVER_H
#define VAR_RESOLVER_H
#include "data.h"
#include "stack_frame.h"

static data* resolve_variable(const char* identifier){
    for (int i = lang::interpreter::stack->size()-1; i >= 0; i--) {
        stack_frame* frame =  (*lang::interpreter::stack)[i];
        auto result = frame->get_data(identifier);
        if(result) {
            return result;
        }
    }
    return nullptr;
}

static proc_dat* resolve_proc(const std::string &name) {
    proc_dat* found = nullptr;
    for (stack_frame* frame : *lang::interpreter::stack) {
        auto result = frame->resolve_proc(name);
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

static void push_stackframe(stack_frame* new_frame) {
    new_frame->set("stack_frame", new data(new long(new_frame->get_id()), "long"));
    lang::interpreter::stack->push_back(new_frame);
}

static void push_stackframe() {
    push_stackframe(new stack_frame());
}
#endif //VAR_RESOLVER_H
