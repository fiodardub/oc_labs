#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

bool runProcess(const std::string& path, const std::vector<int>& args) {
    if (access(path.c_str(), X_OK) != 0) {
        std::cerr << "Ошибка: Исполняемый файл не найден или нет прав на запуск: " << path << "\n";
        return false;
    }
    
    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "Ошибка при вызове fork()\n";
        return false;
    }
    
    else if (pid == 0) {
        std::vector<std::string> string_args;
                string_args.reserve(args.size());
                for (const auto& arg : args) {
                    string_args.push_back(std::to_string(arg));
                }
        
        std::vector<char*> c_args;
        c_args.push_back(const_cast<char*>(path.c_str()));
        for (const auto& arg : string_args) {
            c_args.push_back(const_cast<char*>(arg.c_str()));
        }
        c_args.push_back(nullptr);

        execv(path.c_str(), c_args.data());
        
        std::cerr << "Ошибка при выполнении execv для " << path << "\n";
        exit(1);
    }
    else {
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return true;
        } else {
            std::cerr << "Дочерний процесс " << path << " завершился с ошибкой.\n";
            return false;
        }
    }
}

int main() {
    std::vector<int> vec;
    int n{};
    int x{};
    std::cout << "Output amount: ";
    std::cin >> n; std::cout << std::endl;
    std::cout << "Input elements: " << std::endl;
    for(size_t i{}; i < n; ++i){
        std::cout << "Input " << i+1 << " element: ";
        std::cin >> x; std::cout << std::endl;
        vec.push_back(x);
    }
    std::cout << "Running min_max...\n";
    runProcess("./min_max", vec);
    std::cout << "Running average...\n";
    runProcess("./average", vec);
    
    return 0;
}
