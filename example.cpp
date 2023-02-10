#include <iostream>
#include "../ringbuffer.hpp"

int main()
{
    std::string instr = "Hello World!";
    std::string outstr = "";

    Ringbuffer<char, 16> rbuf;

    rbuf << instr;

    for (char c : rbuf.readAll()) {
        std::cout << c;
    }

    return 0;
}