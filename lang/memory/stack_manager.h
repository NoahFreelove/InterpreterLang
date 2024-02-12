#ifndef VAR_RESOLVER_H
#define VAR_RESOLVER_H
#include "data.h"
static data* resolve_variable(const char* identifier){
    char* name = lang::interpreter::const_char_convert(identifier);
    data* found = nullptr;
    for (stack_frame* frame : *lang::interpreter::stack) {
        auto result = frame->get_data(name);
        if(result) {
            found = result;
        }
    }

    return found;
}

static bool does_variable_exist(char* identifier) {
    for (stack_frame* frame : *lang::interpreter::stack) {
        if(frame->exists(lang::interpreter::const_char_convert(identifier)))
            return true;
    }
    return false;
}

static void assign_variable(char* identifier1, char* identifier2) {
    for (stack_frame* frame : *lang::interpreter::stack) {
        if(frame->assign(identifier1,identifier2))
            break;
    }
}

static void delete_variable(const char* identitifer) {
    for (stack_frame* frame : *lang::interpreter::stack) {
        frame->delete_var(lang::interpreter::const_char_convert(identitifer));
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
    lang::interpreter::stack->push_back(new_frame);
}
#endif //VAR_RESOLVER_H
