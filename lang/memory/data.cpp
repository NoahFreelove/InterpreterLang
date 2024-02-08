#include "data.h"
#include "../interpreter.h"

void data::set_value_int(int val) {

    if(!is_ptr)
        *(int*)value = val;
    else {
        value = new int(val);
    }
}

void data::set_value_float(float val) {
    if(!is_ptr)
        *(float*)value = val;
    else {
        value = new float(val);
    }
}

void data::set_value_double(double val) {
    if(!is_ptr)
        *(double*)value = val;
    else {
        value = new double(val);
    }
}

void data::set_value_long(long val) {
    if(!is_ptr)
        *(long*)value = val;
    else {
        value = new long(val);
    }
}

void data::set_value_char(char val) {
    if(!is_ptr)
        *(char*)value = val;
    else {
        value = new char(val);
    }
}

void data::set_value_string(const std::string& val) {
    if (type != "string") {
        lang::interpreter::error("Type is not string, cannot set.");
        return;
    }

    if(!is_ptr)
        *(std::string*)value = val;
    else {
        value = new std::string(val);
    }
}

void data::set_value_bool(bool val) {
    if(!is_ptr)
        *(bool*)value = val;
    else {
        value = new bool(val);
    }
}

void data::set_value_ulonglong(unsigned long long val) {
    if(!is_ptr)
        *(unsigned long long*)value = val;
    else {
        value = new unsigned long long(val);
    }
}