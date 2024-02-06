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

    bool set(char* identifier, data* val);

    data* get(char* identifier);

    ~stack_memory() {
        for (auto& [key, value] : *memory) {
            delete value;
        }
        delete memory;
    }

    void delete_var(char * identifier);
};



#endif //STACK_MEMORY_H
