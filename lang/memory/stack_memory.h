#ifndef STACK_MEMORY_H
#define STACK_MEMORY_H
#include <unordered_map>
#include "data.h"

class stack_memory {
    std::unordered_map<std::string, data*>* memory;

public:
    stack_memory() {
        memory = new std::unordered_map<std::string, data*>();
    }

    bool set(const char* identifier, data* val);

    // Assign assigns a pointer to a variable
    // use when assigning an identifier to another identifier
    // marks the data as a pointer so its not deleted which would lead to double free
    bool assign(const char* identifier, data* val);

    data* get(const char* identifier);

    void print_stack_memory();

    ~stack_memory() {
        delete_memory();
        delete memory;
    }

    void delete_var(const char * identifier);

    void delete_memory() {
        for (auto& [key, value] : *memory) {
            delete value;
        }
        memory->clear();
    }

    bool exists(const char * identifier);
};



#endif //STACK_MEMORY_H
