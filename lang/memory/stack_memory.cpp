#include "stack_memory.h"

bool stack_memory::set(char *identifier, data *val) {
    std::string key(identifier);
    if (memory->find(key) != memory->end()) {
        // If the identifier is already in the memory, delete the old value
        delete memory->at(key);
        memory->at(key) = val;
        return true;
    }
    // std::cout << "Setting " << key << std::endl;
    memory->insert({key, val});
    return true;

}

data * stack_memory::get(char *identifier) {
    std::string key(identifier);
    // If the identifier is not found, return nullptr
    if (memory->find(key) == memory->end()) {
        return nullptr;
    }
    return memory->at(key);
}

void stack_memory::delete_var(char *identifier) {
    std::string key(identifier);
    if (memory->find(key) != memory->end()) {
        delete memory->at(key);
        memory->erase(key);
    }
}
