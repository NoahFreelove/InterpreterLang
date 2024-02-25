#include "data.h"
#include "../interpreter.h"
#include "../tokenizer/token.h"
#include "stack_frame.h"

data *data::create_default_from_type(const std::string &type, bool is_array, int size) {
    if(type == "int")
        return new data(new int(0), type, false, false, is_array, size);
    if(type == "long")
        return new data(new long(0), type, false, false, is_array, size);
    if(type == "float")
        return new data(new float(0), type, false, false, is_array, size);
    if(type == "double")
        return new data(new double(0), type, false, false, is_array, size);
    if(type == "string")
        return new data(new std::string(""), type, false, false, is_array, size);
    if(type == "ulonglong")
        return new data(new unsigned long long(0), type, false, false, is_array, size);
    if(type == "bool")
        return new data(new bool(false), type, false, false, is_array, size);
    if (type == "char")
        return new data(new char(0), type, false, false, is_array, size);
    std::cout << "Invalid type: " << type << std::endl;
    return nullptr;
}

data *data::create_default_from_type(int type, std::vector<data*>& allocated)  {
    switch (type) {
        case INT:
            return new data(new int(0), "int", allocated);
        case LONG:
            return new data(new long(0), "long", allocated);
        case FLOAT:
            return new data(new float(0), "float", allocated);
        case DOUBLE:
            return new data(new double(0), "double", allocated);
        case STRING:
            return new data(new std::string(""), "string", allocated);
        case ULONG64:
            return new data(new unsigned long long(0), "ulonglong", allocated);
        case BOOL_KEYW:
            return new data(new bool(false), "bool", allocated);
        case CHAR_KEYW:
            return new data(new char(0), "char", allocated);
        default:
            return nullptr;
    }
}

data * data::create_recursive_dimensional_array(std::queue<int> sizes, int type) {
    int length = sizes.front();
    sizes.pop();
    //std::cout << "Length: " << length << std::endl;
    std::string typestr = get_type_as_string(type);
    if(sizes.empty()) {
        return create_default_from_type(typestr, true, length);
    }

    std::vector<data*> pre_allocated;
    for (int i = 0; i < length; ++i) {
        auto* dat = create_recursive_dimensional_array(sizes, type);
        if(!dat) {
            lang::interpreter::error("Invalid type for array");
            return nullptr;
        }
        pre_allocated.push_back(dat);
    }
    return create_default_from_type(type, pre_allocated);


}

data::data(void *value, const std::string &type, const std::vector<data *>& pre_allocated) {
    this->value = value;
    this->type = type;
    this->is_ptr = false;
    this->is_final = false;
    this->type_int = type_registry::reverse_lookup(type);
    this->is_arr = true;
    array_elements = new std::vector<data*>();
    array_elements->reserve(pre_allocated.size());
    for (auto* dat : pre_allocated) {
        array_elements->push_back(dat);
    }

}

data::data(void *value, const std::string &type, bool is_ptr, bool is_final, bool is_array, int initial_size)  {
    this->value = value;
    this->type = type;
    this->is_ptr = is_ptr;
    this->is_final = is_final;
    this->type_int = type_registry::reverse_lookup(type);
    if(is_array) {
        this->is_arr = true;
        array_elements = new std::vector<data*>();
        for (int i = 0; i < initial_size; ++i) {
            auto* dat = create_default_from_type(type);
            if(!dat) {
                lang::interpreter::error("Invalid type for array");
                break;
            }
            array_elements->push_back(dat);
        }
    }
}

void data::set_value_int(int val) {
    if(is_final) {
        lang::interpreter::error("Cannot assign final variable to new value");
        return;
    }
    if(!is_ptr)
        *(int*)value = val;
    else {
        value = new int(val);
    }
}

void data::set_value_float(float val) {
    if(is_final) {
        lang::interpreter::error("Cannot assign final variable to new value");
        return;
    }
    if(!is_ptr)
        *(float*)value = val;
    else {
        value = new float(val);
    }
}

void data::set_value_double(double val) {
    if(is_final) {
        lang::interpreter::error("Cannot assign final variable to new value");
        return;
    }
    if(!is_ptr)
        *(double*)value = val;
    else {
        value = new double(val);
    }
}

void data::set_value_long(long val) {
    if(is_final) {
        lang::interpreter::error("Cannot assign final variable to new value");
        return;
    }
    if(!is_ptr)
        *(long*)value = val;
    else {
        value = new long(val);
    }
}

void data::set_value_char(char val) {
    if(is_final) {
        lang::interpreter::error("Cannot assign final variable to new value");
        return;
    }
    if(!is_ptr)
        *(char*)value = val;
    else {
        value = new char(val);
    }
}

void data::set_value_string(const std::string& val) {
    if(is_final) {
        lang::interpreter::error("Cannot assign final variable to new value");
        return;
    }
    if (type != "string") {
        //lang::interpreter::error("Type is not string, cannot set.");
    }

    if(!is_ptr)
        *(std::string*)value = val;
    else {
        value = new std::string(val);
    }
}

void data::set_value_bool(bool val) {
    if(is_final) {
        lang::interpreter::error("Cannot assign final variable to new value");
        return;
    }
    if(!is_ptr)
        *(bool*)value = val;
    else {
        value = new bool(val);
    }
}

void data::set_value_ulonglong(unsigned long long val) {
    if(is_final) {
        lang::interpreter::error("Cannot assign final variable to new value");
        return;
    }
    if(!is_ptr)
        *(unsigned long long*)value = val;
    else {
        value = new unsigned long long(val);
    }
}

int data::get_type_int() {
    return type_int;
}

data * data::get_array_element(int index) {
    if(!is_arr) {
        lang::interpreter::error("Not an array");
        return nullptr;
    }
    if(index < 0 || index >= array_elements->size()) {
        lang::interpreter::error("Index out of bounds");
        return nullptr;
    }
    return array_elements->at(index);
}

std::string data::get_type_as_string(int t) {
    switch (t) {
        case INT:
            return "int";
        case LONG:
            return "long";
        case FLOAT:
            return "float";
        case DOUBLE:
            return "double";
        case STRING:
            return "string";
        case ULONG64:
            return "ulonglong";
        case BOOL_KEYW:
            return "bool";
        case CHAR_KEYW:
            return "char";
        case TRUE:
            return "true";
        case FALSE:
            return "false";
        default:
            return "unknown";
    }
}
