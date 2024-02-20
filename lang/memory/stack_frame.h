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

    bool set(const std::string& identifier, data* val);
    bool exists(const std::string& identifier);
    bool assign(const std::string& identifier, const std::string& identifier2);

    void delete_var(const std::string &identifier, bool force = false);

    data* get_data(const std::string& identifier);

    void insert_proc(const std::string &name, int type, proc_tokens* p, proc_type_vec* v);

    proc_dat* resolve_proc(const std::string& name);

    static void eval_proc(std::shared_ptr<token_group>& g);

    long get_id() const {
        return id;
    }

    ~stack_frame() {
        delete memory;
    }
};

#endif //STACK_FRAME_H
