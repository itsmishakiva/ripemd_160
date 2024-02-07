#include <iostream>
#include <vector>
#include "ripemd_160.h"

#define vector std::vector
#define string std::string

int main() {
    string input;
    getline(std::cin, input);
    std::cout << Ripemd160::encode(input);
    return 0;
}
