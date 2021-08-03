#include <iostream>
#include "Grabber.hpp"

int main()
{
    // Get tokens
    std::vector<std::string> tokens = grabber::grab_tokens();

    // Print
    for (auto& token : tokens)
        std::cout << token << "\n";

    std::cin.get(); // Block until input
    return 0;
}