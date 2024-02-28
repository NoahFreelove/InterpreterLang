#ifndef BUILT_IN_RUNNER_H
#define BUILT_IN_RUNNER_H
#include "../tokenizer/token.h"
#include <memory>
#include <vector>

void define(const std::vector<std::shared_ptr<token>> &tokens);

void undefine(const std::vector<std::shared_ptr<token>> &tokens);

void print(const std::vector<std::shared_ptr<token>>& tokens, int offset = 1);

void process_import(std::vector<std::shared_ptr<token>> tokens);

void dump();
void assert(std::vector<std::shared_ptr<token>> tokens);

typedef std::pair<std::shared_ptr<token>,std::shared_ptr<token>> proc_type;
typedef std::vector<proc_type> proc_type_vec;

void execute_internal_method(const std::string& proc_name, proc_type_vec args);

void run_builtins(const std::vector<std::shared_ptr<token>>& tokens);
#endif //BUILT_IN_RUNNER_H
