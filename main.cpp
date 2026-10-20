#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdlib>
#include <ctime>
#include "marker_logic.h"


int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    int num_arguments = 0;
    int num_threads = 0;

    std::cout << "Введите количество аргументов (размер массива): ";
    std::cin >> num_arguments;
    std::cout << "Количество запросов marker (потоков): ";
    std::cin >> num_threads;

    if (num_arguments <= 0 || num_threads <= 0) {
        std::cout << "Размер массива и количество потоков должны быть положительными.\n";
        return 1;
    }

    std::vector<int> shared_array(num_arguments, 0);
    reset_sync_state(num_threads);

    std::vector<std::thread> workers;
    for (int i = 1; i <= num_threads; ++i) {
        workers.emplace_back(marker, std::ref(shared_array), i);
    }

    for (int i = 1; i <= num_threads; ++i) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            active_marker_queue = i;
        }
        cv_from_main.notify_all();

        {
            std::unique_lock<std::mutex> lock(mtx);
            cv_to_main.wait(lock, [] { return ready_for_main; });
            ready_for_main = false;
            std::cout << "[Main] Перехватил управление от потока " << active_signaling_queue << "\n\n";
        }
    }

    while (true) {
        int alive_count = 0;
        for (int i = 0; i < num_threads; ++i) {
            if (!thread_should_terminate[i]) alive_count++;
        }
        if (alive_count == 0) break;

        std::cout << "\n[Main] Текущее содержимое массива:\n";
        {
            std::lock_guard<std::mutex> lock(mtx);
            for (int val : shared_array) {
                std::cout << val << " ";
            }
            std::cout << "\n\n";
        }

        int target_queue;
        std::cout << "Введите номер потока для ЗАВЕРШЕНИЯ (от 1 до " << num_threads << "): ";
        std::cin >> target_queue;

        if (target_queue < 1 || target_queue > num_threads || thread_should_terminate[target_queue - 1]) {
            std::cout << "Некорректный номер или поток уже завершен!\n";
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            thread_should_terminate[target_queue - 1] = true;
        }
        cv_from_main.notify_all();

        workers[target_queue - 1].join();

        std::cout << "[Main] Массив после удаления потока " << target_queue << ":\n";
        {
            std::lock_guard<std::mutex> lock(mtx);
            for (int val : shared_array) {
                std::cout << val << " ";
            }
            std::cout << "\n";
        }

        alive_count = 0;
        for (int i = 0; i < num_threads; ++i) {
            if (!thread_should_terminate[i]) alive_count++;
        }
        if (alive_count == 0) break;

        std::cout << "[Main] Подаю сигнал на продолжение работы оставшимся потокам...\n";
        {
            std::lock_guard<std::mutex> lock(mtx);
            for (int i = 0; i < num_threads; ++i) {
                if (!thread_should_terminate[i]) {
                    thread_stopped_working[i] = false;
                }
            }
        }

        for (int i = 1; i <= num_threads; ++i) {
            if (thread_should_terminate[i - 1]) continue;

            {
                std::lock_guard<std::mutex> lock(mtx);
                active_marker_queue = i;
            }
            cv_from_main.notify_all();

            {
                std::unique_lock<std::mutex> lock(mtx);
                cv_to_main.wait(lock, [] { return ready_for_main; });
                ready_for_main = false;
                std::cout << "[Main] Перехватил управление от продолжающего потока " << active_signaling_queue << "\n";
            }
        }
    }

    std::cout << "\n[Main] Все потоки-маркеры успешно завершили работу. Программа окончена.\n";
    return 0;
}
