#include <gtest/gtest.h>
#include <algorithm>
#include "marker_logic.h"


class MarkerLabTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::srand(42);
    }
};

TEST_F(MarkerLabTest, SingleThreadMarksElementsAndStopsOnCollision) {
    const int array_size = 5;
    const int num_threads = 1;
    reset_sync_state(num_threads);

    std::vector<int> shared_array(array_size, 0);
    std::thread worker(marker, std::ref(shared_array), 1);

    {
        std::lock_guard<std::mutex> lock(mtx);
        active_marker_queue = 1;
    }
    cv_from_main.notify_all();
    
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv_to_main.wait(lock, [] { return ready_for_main; });
        EXPECT_EQ(active_signaling_queue, 1);
        EXPECT_TRUE(thread_stopped_working[0]);
    }

    int marked_count = std::count(shared_array.begin(), shared_array.end(), 1);
    EXPECT_GT(marked_count, 0);

    {
        std::lock_guard<std::mutex> lock(mtx);
        thread_should_terminate[0] = true;
    }
    cv_from_main.notify_all();
    worker.join();
}

TEST_F(MarkerLabTest, ThreadClearsItsMarksUponTermination) {
    const int array_size = 6;
    const int num_threads = 1;
    reset_sync_state(num_threads);

    std::vector<int> shared_array(array_size, 0);
    std::thread worker(marker, std::ref(shared_array), 1);

    {
        std::lock_guard<std::mutex> lock(mtx);
        active_marker_queue = 1;
    }
    cv_from_main.notify_all();

    {
        std::unique_lock<std::mutex> lock(mtx);
        cv_to_main.wait(lock, [] { return ready_for_main; });
        ready_for_main = false;
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        thread_should_terminate[0] = true;
    }
    cv_from_main.notify_all();
    worker.join();

    for (int val : shared_array) {
        EXPECT_EQ(val, 0);
    }
}

TEST_F(MarkerLabTest, BoundarySingleElementArrayWithTwoThreads) {
    const int array_size = 1;
    const int num_threads = 2;
    reset_sync_state(num_threads);

    std::vector<int> shared_array(array_size, 0);
    std::thread w1(marker, std::ref(shared_array), 1);
    std::thread w2(marker, std::ref(shared_array), 2);

    {
        std::lock_guard<std::mutex> lock(mtx);
        active_marker_queue = 1;
    }
    cv_from_main.notify_all();
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv_to_main.wait(lock, [] { return ready_for_main; });
        ready_for_main = false;
    }
    EXPECT_EQ(shared_array[0], 1);

    {
        std::lock_guard<std::mutex> lock(mtx);
        active_marker_queue = 2;
    }
    cv_from_main.notify_all();
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv_to_main.wait(lock, [] { return ready_for_main; });
        ready_for_main = false;
    }
    EXPECT_EQ(shared_array[0], 1);

    {
        std::lock_guard<std::mutex> lock(mtx);
        thread_should_terminate[0] = true;
        thread_should_terminate[1] = true;
    }
    cv_from_main.notify_all();
    w1.join();
    w2.join();

    EXPECT_EQ(shared_array[0], 0);
}

TEST_F(MarkerLabTest, RemainingThreadContinuesAfterOtherTerminates) {
    const int array_size = 2;
    const int num_threads = 2;
    reset_sync_state(num_threads);

    std::vector<int> shared_array(array_size, 0);
    std::thread w1(marker, std::ref(shared_array), 1);
    std::thread w2(marker, std::ref(shared_array), 2);

    for (int i = 1; i <= num_threads; ++i) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            active_marker_queue = i;
        }
        cv_from_main.notify_all();

        std::unique_lock<std::mutex> lock(mtx);
        cv_to_main.wait(lock, [] { return ready_for_main; });
        ready_for_main = false;
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        thread_should_terminate[0] = true;
    }
    cv_from_main.notify_all();
    w1.join();

    {
        std::lock_guard<std::mutex> lock(mtx);
        thread_stopped_working[1] = false;
        active_marker_queue = 2;
    }
    cv_from_main.notify_all();

    {
        std::unique_lock<std::mutex> lock(mtx);
        cv_to_main.wait(lock, [] { return ready_for_main; });
        ready_for_main = false;
    }
    
    int count_two = std::count(shared_array.begin(), shared_array.end(), 2);
    EXPECT_GE(count_two, 1);

    {
        std::lock_guard<std::mutex> lock(mtx);
        thread_should_terminate[1] = true;
    }
    cv_from_main.notify_all();
    w2.join();
}
