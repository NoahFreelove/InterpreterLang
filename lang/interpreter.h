#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <iostream>
#include <string>
namespace lang {

    class interpreter {

    public:
        static void input_loop();
        static void error(std::string err);

    };
}
#endif //INTERPRETER_H
