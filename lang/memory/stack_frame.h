#ifndef STACK_FRAME_H
#define STACK_FRAME_H
#include "stack_memory.h"
#include "../memory/proc_manager.h"

class stack_frame {
    inline static long offset = 0;
    long id;
    stack_memory* memory;
    proc_manager* procs;
public:
    stack_frame() {
        id = offset++;
        memory = new stack_memory();
        procs = new proc_manager();
    }

    void dump_memory() {
        std::cout << "Dump for stackframe " << id << std::endl  << std::endl << "Variables:" << std::endl;
        memory->print_stack_memory();

        std::cout << std::endl << "Methods:" << std::endl;
        procs->print_procs();
    }

    bool set(const char* identifier, data* val);
    bool exists(const char* identifier);
    bool assign(const char* identifier, const char* identifier2);

    void delete_var(const char* identifier);

    data* get_data(const char* identifier);

    void insert_proc(const std::string &name, proc_tokens* p, proc_type_vec* v);

    proc* resolve_proc(const std::string& name);

    long get_id() const {
        return id;
    }

    ~stack_frame() {
        delete memory;
    }
};

#endif //STACK_FRAME_H
