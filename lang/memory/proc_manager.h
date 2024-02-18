#ifndef PROC_MANAGER_H
#define PROC_MANAGER_H
#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>

#include "../tokenizer/token.h"
#include "../tokenizer/token_group.h"

// TYPEWORD, IDENTIFIER
typedef std::pair<std::shared_ptr<token>,std::shared_ptr<token>> proc_type;
typedef std::vector<proc_type> proc_type_vec;
typedef std::vector<std::vector<std::shared_ptr<token>>> proc_tokens;
// right side of pair is the proc return type
typedef std::pair<std::pair<proc_tokens*, proc_type_vec*>*,int> proc_dat;
struct proc {
    proc_dat dat;
    std::stack<bool> activated_if_blocks = std::stack<bool>();
};
class proc_manager {
    std::unordered_map<std::string, proc_dat*>* procs;
public:
    proc_manager() {
        procs = new std::unordered_map<std::string, proc_dat*>();
    }
    ~proc_manager() {
        // loop through procs and delete each pair element
        for(auto& p : *procs) {
            std::pair<proc_tokens*, proc_type_vec*> pr = *p.second->first;
            delete pr.first;
            delete pr.second;
            delete p.second;
        }
        delete procs;
    }

    proc_dat* resolve_proc_name(const std::string& name);
    void insert_proc(const std::string& name, int type, proc_tokens* p, proc_type_vec* v);

    static void execute_proc(std::shared_ptr<token_group>& g);

    void print_procs();
};
#endif //PROC_MANAGER_H
