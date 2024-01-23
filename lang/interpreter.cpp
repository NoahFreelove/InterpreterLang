#include "interpreter.h"
#include "tokenizer/tokens.h"

void lang::interpreter::input_loop() {
    auto* input = new std::string();
    auto scanner = lang::scanner();

    while (true) {
        if(scanner.in_multi_line()) {
            std::cout << "...";
        }
        else {
            std::cout << ">>>";
        }
        std::getline(std::cin, *input);
        if (*input == "exit") {
            break;
        }
        auto tokens = scanner.scan_line(input);
        for (auto token : tokens) {
            std::cout << *token << std::endl;
        }
    }
}

void lang::interpreter::error(std::string err) {
    std::cerr << "Error: " << err << std::endl;
}
