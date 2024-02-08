#include <iostream>
#include "lang/lang.h"
int main()
{
    lang::interpreter::read_from_file("game.lang");

    token* openp2 = new token(LEFT_PAREN, "(", 0);
    token* openp = new token(LEFT_PAREN, "(", 0);
    token* t = new token(INT, "5", 0, 5);
    token* add = new token(SLASH, "/", 0);
    token* var = new token(IDENTIFIER, "x", 0, 5);
    token* closep = new token(RIGHT_PAREN, ")", 0);
    token* closep2 = new token(RIGHT_PAREN, ")", 0);
    token* mul = new token(STAR, "*", 0);
    token* t3 = new token(INT, "5", 0, 5);
    // Equivalent to (5+x)*5

    //lang::interpreter::stack = new std::stack<stack_frame*>();
    //lang::interpreter::stack->push(lang::interpreter::global_frame);
    //lang::interpreter::global_frame->set(lang::interpreter::const_char_convert("x"), new data(new int(5), "int"));

    //token_group* group = token_grouper::recursive_group({openp, t,mul,t3,closep});
    //group_evaluator::eval_group(group);

    //std::cout << std::any_cast<int>(group->value) << std::endl;


    //std::cout << std::any_cast<int>(final->get_value()) << std::endl;
    //lang::interpreter::input_loop();

    return 0;
}
