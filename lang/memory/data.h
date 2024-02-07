#ifndef DATA_H
#define DATA_H
#include <iostream>
#include <ostream>
#include <string>

class data {
    void* value;
    const char* type;
    bool is_ptr = false;
public:
    data(void* value, const char* type, bool is_ptr = false) {
        this->value = value;
        this->type = type;
        this->is_ptr = is_ptr;
    }

    void* get() {
        return value;
    }

    const char* get_type() {
        return type;
    }

    // Byval keyword gets value and copies it into a new address
    // otherwise all data is passed by pointer (reference)

    int get_int() {
        return *(int*)value;
    }

    float get_float() {
        return *(float*)value;
    }

    double get_double() {
        return *(double*)value;
    }

    long get_long() {
        return *(long*)value;
    }

    char get_char() {
        return *(char*)value;
    }

    char* get_string() {
        return (char*)value;
    }

    bool get_bool() {
        return *(bool*)value;
    }

    unsigned long long get_ulonglong() {
        return *(unsigned long long*)value;
    }

    int* get_int_ref() {
        return (int*)value;
    }

    float* get_float_ref() {
        return (float*)value;
    }

    double* get_double_ref() {
        return (double*)value;
    }

    long* get_long_ref() {
        return (long*)value;
    }

    char* get_char_ref() {
        return (char*)value;
    }

    char** get_string_ref() {
        return (char**)value;
    }

    bool* get_bool_ref() {
        return (bool*)value;
    }

    unsigned long long* get_ulonglong_ref() {
        return (unsigned long long*)value;
    }

    void set_value_int(int val) {
        if(!is_ptr)
            *(int*)value = val;
        else {
            value = new int(val);
        }
    }
    void set_value_float(float val) {
        if(!is_ptr)
            *(float*)value = val;
        else {
            value = new float(val);
        }
    }
    void set_value_double(double val) {
        if(!is_ptr)
            *(double*)value = val;
        else {
            value = new double(val);
        }
    }
    void set_value_long(long val) {
        if(!is_ptr)
            *(long*)value = val;
        else {
            value = new long(val);
        }
    }
    void set_value_char(char val) {
        if(!is_ptr)
            *(char*)value = val;
        else {
            value = new char(val);
        }
    }
    void set_value_string(char* val) {
        if(!is_ptr)
            *(char**)value = val;
        else {
            value = new char*(val);
        }
    }
    void set_value_bool(bool val) {
        if(!is_ptr)
            *(bool*)value = val;
        else {
            value = new bool(val);
        }
    }
    void set_value_ulonglong(unsigned long long val) {
        if(!is_ptr)
            *(unsigned long long*)value = val;
        else {
            value = new unsigned long long(val);
        }
    }



    std::string to_string() {
        if (type == "int") {
            return std::to_string(get_int());
        }
        else if (type == "float") {
            return std::to_string(get_float());
        }
        else if (type == "double") {
            return std::to_string(get_double());
        }
        else if (type == "long") {
            return std::to_string(get_long());
        }
        else if (type == "char") {
            return {get_char()};
        }
        else if (type == "bool") {
            return get_bool() ? "true" : "false";
        }
        else if (type == "string") {
            return {get_string()};
        }
        else if (type == "unsigned long long") {
            return std::to_string(*static_cast<unsigned long long *>(value));
        }
        return "Invalid type";
    }

    bool is_pointer() const {
        return is_ptr;
    }


    ~data() {
        if(is_ptr) {
            std::cout << "Not deleting data of type: " << type << " because it is a pointer" << std::endl;
            return;
        }
        std::cout << "Deleting data of type: " << type << std::endl;
        if (type == "int") {
            delete (int*)value;
        }
        else if (type == "float") {
            delete (float*)value;
        }
        else if (type == "double") {
            delete (double*)value;
        }
        else if (type == "long") {
            delete (long*)value;
        }
        else if (type == "char") {
            delete (char*)value;
        }
        else if (type == "bool") {
            delete (bool*)value;
        }
        else if (type == "string") {
            free(*(char**)value);
            delete (char**)value;
        }
        else if (type == "unsigned long long") {
            delete (unsigned long long*)value;
        }
    }
};
#endif //DATA_H
