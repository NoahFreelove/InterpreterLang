#include "stack_memory.h"

bool stack_memory::set(char *identifier, data *val) {
    std::string key(identifier);
    if (memory->find(key) != memory->end()) {
        // If the identifier is already in the memory, remap old one to name + "_old"
        memory->insert({key + "_old", memory->at(key)});
        memory->at(key) = val;
        return true;
    }
    // std::cout << "Setting " << key << std::endl;
    memory->insert({key, val});
    return true;

}

bool stack_memory::assign(char *identifier, data *val) {
    std::string key(identifier);
    auto* new_dat = new data(val->get(), val->get_type(), true);
    if (memory->find(key) != memory->end()) {
        // If the identifier is already in the memory, remap old one to name + "_old"
        memory->insert({key + "_old", memory->at(key)});
        memory->at(key) = new_dat;
        return true;
    }
    // std::cout << "Setting " << key << std::endl;
    memory->insert({key, new_dat});
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

void stack_memory::print_stack_memory() {
    for (auto& [key, value] : *memory) {
        std::cout << key << " : " << value->to_string() << std::endl;
    }
}

void stack_memory::delete_var(char *identifier) {
    std::string key(identifier);
    if (memory->find(key) != memory->end()) {
        delete memory->at(key);
        memory->erase(key);
    }
}
