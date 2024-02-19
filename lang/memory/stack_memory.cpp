#include "stack_memory.h"

bool stack_memory::set(const std::string& identifier, data *val) {
    std::string key(identifier);
    if (memory->find(key) != memory->end()) {
        // If the identifier is already in the memory, remap old one to name + "_old"
        // Unless the old one is a pointer, which another variable is pointing to
        if (!memory->at(key)->is_pointer()) {
            memory->insert({key + "_old", memory->at(key)});
        }
        memory->at(key) = val;
        return true;
    }
    // std::cout << "Setting " << key << std::endl;
    memory->insert({key, val});
    return true;

}

bool stack_memory::assign(const std::string& identifier, data *val) {
    std::string key(identifier);
    auto* new_dat = new data(val->get(), val->get_type(), true);
    if (memory->find(key) != memory->end()) {
        // Unless the old one is a pointer, which another variable is pointing to
        if (!memory->at(key)->is_pointer()) {
            memory->insert({key + "_old", memory->at(key)});
        }
        memory->at(key) = new_dat;
        return true;
    }
    // std::cout << "Setting " << key << std::endl;
    memory->insert({key, new_dat});
    return true;
}

data * stack_memory::get(const std::string& identifier) {
    std::string key(identifier);
    // If the identifier is not found, return nullptr
    if (memory->find(key) == memory->end()) {
        return nullptr;
    }
    return memory->at(key);
}

void stack_memory::print_stack_memory() {
    for (auto& [key, value] : *memory) {
        std::cout << key << " : Value: " << value->to_string() << " : Address: " << value->get() << std::endl;
    }
    if(memory->empty()) {
        std::cout << "none..." << std::endl;
    }
}

void stack_memory::delete_var(const std::string& identifier) {
    std::string key(identifier);
    if (memory->find(key) != memory->end()) {
        std::cout << "Deleted: " << key << "(type: " << memory->find(key)->second->get_type() << ")" <<std::endl;
        delete memory->at(key);
        memory->erase(key);
    }
}

bool stack_memory::exists(const std::string& identifier) {
    return memory->find(std::string(identifier)) != memory->end();
}
