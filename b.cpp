#include <iostream>

int main(int argc, char* argv[]) {
    if (argc!= 2) {
        std::cerr << "Usage: subprogram <message>" << std::endl;
        return -1;
    }

    std::string message(argv[1]);
    std::cout << "Received message: " << message << std::endl;

    // Your subprogram code here

    return 0;
}