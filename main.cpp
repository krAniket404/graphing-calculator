#include "functions.h"

#include <iostream>

int main() {
    Graph graph;

    std::cout << "Enter a function of x: ";

    if (!(std::cin >> graph)) {
        std::cerr << "Invalid function or input error.\n";
        return 1;
    }

    std::cout << graph;

    return 0;
}