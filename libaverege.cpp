#include <vector>
#include <thread>
#include <chrono>

extern "C" double find_average(const std::vector<int>& vec) {
    double sum{};
    
    for (const auto& x : vec) {
        sum += x;
        std::this_thread::sleep_for(std::chrono::milliseconds(12));
    }
    
    if (vec.empty()) {
        return 0.0;
    }
    
    sum /= vec.size();
    return sum;
}

