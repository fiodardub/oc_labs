#include <vector>
#include <iostream>
#include <cstdlib>
#include <dlfcn.h>

typedef double (*FindAverageFunc)(const std::vector<int>&);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Using: " << argv[0] << " <number1> <number2> ...\n";
        return 1;
    }
    
    std::vector<int> vec;
    for (int i = 1; i < argc; ++i) {
        vec.push_back(std::atoi(argv[i]));
    }
    
    double aver = 0.0;
    
    void* handle = dlopen("./libaverege.dylib", RTLD_LAZY);
    if (!handle) {
        std::cerr << "[Average Process] Ошибка dlopen: " << dlerror() << "\n";
        return 1;}
        
        FindAverageFunc find_average = (FindAverageFunc)dlsym(handle, "find_average");
        const char* dlsym_error = dlerror();
        if (dlsym_error) {
            std::cerr << "[Average Process] Ошибка dlsym: " << dlsym_error << "\n";
            dlclose(handle);
            return 1;
        }
        
        aver = find_average(vec);
        
        dlclose(handle);
        
        std::cout << "Average of our array: " << aver << "\n";
        return 0;
}

