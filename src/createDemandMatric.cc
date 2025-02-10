#include <vector>
#include <chrono>
#include <iostream>


int main (int argc, char* argv[]) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    srand(seed);
}