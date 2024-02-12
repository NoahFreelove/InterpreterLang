#ifndef STACK_FRAME_H
#define STACK_FRAME_H
#include "stack_memory.h"

class stack_frame {
    inline static long offset = 0;
    long id;
    stack_memory* memory;
public:
    stack_frame() {
        id = offset++;
        memory = new stack_memory();
    }

    void dump_memory() {
        std::cout << "Dump for stackframe " << id << std::endl;
        memory->print_stack_memory();
    }

    bool set(char* identifier, data* val);
    bool exists(char* identifier);
    bool assign(char* identifier, char* identifier2);

    void delete_var(char* identifier);

    data* get_data(char* identifier);

    long get_id() {
        return id;
    }


    ~stack_frame() {
        delete memory;
    }
};

#endif //STACK_FRAME_H
