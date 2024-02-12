#include <iostream>
#include "lang/lang.h"
int main()
{
    //lang::interpreter::read_from_file("file.lang");
    lang::interpreter::input_loop();
    return 0;
}
