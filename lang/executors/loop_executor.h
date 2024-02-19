#ifndef LOOP_EXECUTOR_H
#define LOOP_EXECUTOR_H
#include "../memory/proc_manager.h"
typedef proc_tokens loop_data;
struct loop{
    int type = 0;

    loop_data* loop_lines = nullptr;
    std::vector<std::shared_ptr<token>>* condition = nullptr;
    std::shared_ptr<token> iterator_variable = nullptr;

    ~loop() {
        if(loop_lines)
            delete loop_lines;
        if (condition)
            delete condition;

    }
};

class loop_executor{
    std::vector<loop*> active_loops = std::vector<loop*>();


};

#endif //LOOP_EXECUTOR_H
