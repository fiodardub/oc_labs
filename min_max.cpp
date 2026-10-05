#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>

int find_min(const std::vector<int>& arr) {
    if (arr.empty()) return 0;
    int x = arr[0];
    for (const auto& a : arr) {
        if (x > a) {
            std::this_thread::sleep_for(std::chrono::milliseconds(7));
            x = a;
        }
    }
    return x;
}

int find_max(const std::vector<int>& arr) {
    if (arr.empty()) return 0;
    int x = arr[0];
    for (const auto& a : arr) {
        if (x < a) {
            std::this_thread::sleep_for(std::chrono::milliseconds(7));
            x = a;
        }
    }
    return x;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Using: " << argv[0] << " <number1> <number2> ...\n";
        return 1;
    }

    std::vector<int> vec;
    vec.reserve(argc - 1);
    
    for (int i = 1; i < argc; ++i) {
        vec.push_back(std::atoi(argv[i]));
    }
    
    int min = find_min(vec);
    int max = find_max(vec);
    
    std::cout << "Min element: " << min << std::endl;
    std::cout << "Max element: " << max << std::endl;
    
    return 0;
}
