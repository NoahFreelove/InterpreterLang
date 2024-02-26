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
    proc_dat* dat;
    bool is_native;
    proc() : dat(nullptr), is_native(false){}

    explicit proc(proc_dat* dat, bool is_native = false) {
        this->dat = dat;
        this->is_native = is_native;
    }
};
class proc_manager {
    std::unordered_map<std::string, std::vector<proc>>* procs;
public:
    proc_manager() {
        procs = new std::unordered_map<std::string, std::vector<proc>>();
    }
    ~proc_manager() {
        // loop through procs and delete each pair element
        for(auto& pro : *procs) {
            auto p = pro.second;
            for (auto dat : p) {
                std::pair<proc_tokens*, proc_type_vec*> pr = *dat.dat->first;
                delete pr.first;
                delete pr.second;
                delete dat.dat;
            }

        }
        delete procs;
    }

    bool same_types(proc_type_vec& a, proc_type_vec& b);

    proc_dat* resolve_proc_name(const std::string& name, proc_type_vec& types);
    void insert_proc(const std::string& name, int type, proc_tokens* p, proc_type_vec* v);

    bool exists(const std::string & name);

    static void execute_proc(std::shared_ptr<token_group>& g);

    void print_procs();

    inline static proc_tokens* new_proc_tokens = nullptr;
    inline static proc_type_vec* types = nullptr;
    inline static long proc_stack_id = -1L;
    inline static int proc_type = NOTHING;
    inline static std::string proc_name;

    static void process_return(const std::vector<std::shared_ptr<token>> &tokens, int offset);
    static void process_proc_declaration(std::vector<std::shared_ptr<token>> &tokens);
    static void end_proc_declaration();
};
#endif //PROC_MANAGER_H
