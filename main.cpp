#include <iostream>
#include "lang/lang.h"
int main()
{
    //lang::interpreter::read_from_file("file.lang");

    token* openp = new token(LEFT_PAREN, "(", 0);
    token* t = new token(INT, "5", 0, 5);
    token* add = new token(PLUS, "+", 0);
    token* var = new token(IDENTIFIER, "x", 0, 5);
    token* closep = new token(RIGHT_PAREN, ")", 0);
    token* mul = new token(STAR, "*", 0);
    token* t3 = new token(INT, "5", 0, 5);
    // Equivalent to (5+x)*5

    lang::interpreter::stack = new std::stack<stack_frame*>();
    lang::interpreter::stack->push(lang::interpreter::global_frame);
    lang::interpreter::global_frame->set(lang::interpreter::const_char_convert("x"), new data(new int(5), "int"));


    token_group* group = token_grouper::recursive_group({openp,t,add,var,closep,mul,t3});
    group_evaluator::eval_group(group);

    group->print_group();

    //lang::interpreter::input_loop();
    return 0;
}
