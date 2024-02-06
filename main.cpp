#include <iostream>
#include "lang/lang.h"
int main()
{
    //lang::interpreter::read_from_file("file.lang");
    token* openp = new token(LEFT_PAREN, "(", 0);
    token* t = new token(INT, "5", 0, 5);
    token* add = new token(PLUS, "+", 0);
    token* t2 = new token(INT, "5", 0, 5);
    token* closep = new token(RIGHT_PAREN, ")", 0);
    token* mul = new token(STAR, "*", 0);
    token* t3 = new token(INT, "5", 0, 5);

    lang::interpreter::input_loop();
    return 0;
}
