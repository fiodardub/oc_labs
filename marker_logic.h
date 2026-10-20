#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdlib>
#include <ctime>

inline std::mutex mtx;
inline std::condition_variable cv_to_main;
inline std::condition_variable cv_from_main;

inline int active_marker_queue = -1;
inline bool ready_for_main = false;
inline int active_signaling_queue = -1;

inline std::vector<bool> thread_stopped_working;
inline std::vector<bool> thread_should_terminate;


inline void reset_sync_state(int num_threads) {
    std::lock_guard<std::mutex> lock(mtx);
    active_marker_queue = -1;
    ready_for_main = false;
    active_signaling_queue = -1;
    thread_stopped_working.assign(num_threads, false);
    thread_should_terminate.assign(num_threads, false);
}

inline void marker(std::vector<int>& args, int queue) {
    size_t size = args.size();
    size_t sum = 0;

    while (true) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv_from_main.wait(lock, [queue] {
                return active_marker_queue == queue || thread_should_terminate[queue - 1];
            });

            if (thread_should_terminate[queue - 1]) {
                for (size_t i = 0; i < size; ++i) {
                    if (args[i] == queue) {
                        args[i] = 0;
                    }
                }
                break;
            }
            active_marker_queue = -1;
            sum = 0;
        }

        while (true) {
            int x = rand() % static_cast<int>(size);
            std::unique_lock<std::mutex> lock(mtx);

            if (args[x] == 0) {
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(5));

                lock.lock();
                if (args[x] == 0) {
                    args[x] = queue;
                    ++sum;
                }
                lock.unlock();

                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            } else {
                ready_for_main = true;
                active_signaling_queue = queue;
                thread_stopped_working[queue - 1] = true;

                lock.unlock();
                cv_to_main.notify_one();
                break;
            }
        }
    }
}
