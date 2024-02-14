#ifndef PROC_MANAGER_H
#define PROC_MANAGER_H
#include <memory>
#include <vector>
#include "../tokenizer/token.h"
#include "../tokenizer/token_group.h"
typedef std::pair<std::shared_ptr<token>,std::shared_ptr<token>> proc_type;
typedef std::vector<proc_type> proc_type_vec;
typedef std::vector<std::vector<std::shared_ptr<token>>> proc_tokens;
typedef std::pair<proc_tokens*, proc_type_vec*> proc;
class proc_manager {
    std::unordered_map<std::string, proc*>* procs;
public:
    proc_manager() {
        procs = new std::unordered_map<std::string, std::pair<proc_tokens*, proc_type_vec*>*>();
    }
    ~proc_manager() {
        // loop through procs and delete each pair element
        for(auto& p : *procs) {
            std::pair<proc_tokens*, proc_type_vec*> pr = *p.second;
            delete pr.first;
            delete pr.second;
            delete p.second;
        }
        delete procs;
    }

    proc* resolve_proc_name(const std::string& name) {
        // return nullptr if not in procs
        if(procs->find(name) == procs->end()) {
            return nullptr;
        }
        return (*procs)[name];
    }
    void insert_proc(std::string name, proc_tokens* p, proc_type_vec* v) {
        procs->insert(std::make_pair(name, new std::pair(p,v)));
    }

    void execute_proc(std::shared_ptr<token_group>& g) {
        //std::vector<std::shared_ptr<token_group>> args = g.

        g->type = NOTHING;
        g->value = nullptr;
    }

    void print_procs() {
        for (const auto& elm : *procs) {
            std::cout << elm.first << "(";
            int i = 0;
            for (const auto& t : (*elm.second->second)) {
                std::cout << id_to_name(t.first->get_name()) << " " << t.second->get_lexeme();
                i++;
                if(i < elm.second->second->size()) {
                    std::cout << ", ";
                }
                else {
                    std::cout << ')' << std::endl;
                }
            }
        }
        if(procs->empty())
            std::cout << "none..." << std::endl;
    }
};
#endif //PROC_MANAGER_H
