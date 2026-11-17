#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <thread>
#include <dlfcn.h>

typedef double (*FindAverageFunc)(const std::vector<int>&);

std::ofstream fout("/Users/h/oc_lab_2/oc_lab_2/average.txt");

bool runThreadParallel(void(*func)(const std::vector<int>&), const std::vector<int>& args, std::vector<std::thread>& thread_pool) {
    try {
        thread_pool.emplace_back(func, args);
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при создании потока: " << e.what() << "\n";
        return false;
    }
}

void min_max(const std::vector<int>& vec){
    if (vec.empty()) return ;
    int x_min = vec[0], x_max = vec[0];
    for (const auto& a : vec) {
        if (x_min > a) {
            std::this_thread::sleep_for(std::chrono::milliseconds(7));
            x_min = a;
        }
    }
    for (const auto& a : vec) {
        if (x_max < a) {
            std::this_thread::sleep_for(std::chrono::milliseconds(7));
            x_max = a;
        }
    }
    
    std::cout << "Min: " << x_min << " " << "Max: " << x_max << '\n';
    
    fout << x_min << ' ' << x_max << ' ';
}

void average(const std::vector<int>& vec){
    double aver = 0.0;
    
    void* handle = dlopen("/Users/h/oc_lab_2/oc_lab_2/libaverege.dylib", RTLD_LAZY);
    if (!handle) {
        std::cerr << "[Average Process] Ошибка dlopen: " << dlerror() << "\n";
        return ;
    }
    
    FindAverageFunc find_average = (FindAverageFunc)dlsym(handle, "find_average");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "[Average Process] Ошибка dlsym: " << dlsym_error << "\n";
        dlclose(handle);
        return ;
    }
    
    aver = find_average(vec);
    
    dlclose(handle);
    
    std::cout << "Average: " << aver << '\n';
    
    fout << aver;
}

void func(std::vector<int>& arr, const std::vector<double>& temp){
    if (temp.size() < 3) return;
    
    int aver = static_cast<int>(temp[2] + 0.5);
    int x_min = static_cast<int>(temp[0]);
    int x_max = static_cast<int>(temp[1]);
    
    long long i_min = -1, i_max = -1;
    size_t size = arr.size();
    
    for(size_t i = 0; i < size; ++i){
        if(x_min == arr[i]){
            i_min = i;
        }
        if(x_max == arr[i]){
            i_max = i;
        }
    }
    
    if (i_min != -1) arr[i_min] = aver;
    if (i_max != -1) arr[i_max] = aver;
}

int main() {
    std::vector<std::thread> storage;
    std::vector<int> vec;
    int n{};
    int x{};
    std::cout << "Output amount: ";
    std::cin >> n; std::cout << std::endl;
    std::cout << "Input elements: " << std::endl;
    for(size_t i = 0; i < n; ++i){
        std::cout << "Input " << i+1 << " element: ";
        std::cin >> x; std::cout << std::endl;
        vec.push_back(x);
    }
    
    runThreadParallel(min_max, vec, storage);
    runThreadParallel(average, vec, storage);
    
    for (auto& th : storage) {
        if (th.joinable()) {
            th.join();
        }
    }
    
    fout.close();
    
    std::ifstream fin("/Users/h/oc_lab_2/oc_lab_2/average.txt");
    std::vector<double> temp;
    double value;
    for(size_t i = 0; i < 3; ++i){
        fin >> value;
        temp.push_back(value);
    }
    
    func(vec,temp);
    
    std::cout << "Changed array: " << "\n";
    for(const auto& x: vec){
        std::cout << x << " ";
    }
    
    return 0;
}
