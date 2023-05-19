#include "Workers.h"
#include <thread>
#include <condition_variable>

void Workers::post(const std::function<void()>& task) {
    post_timeout(task, 0);
}

void Workers::post_timeout(const std::function<void()>& task, const int milliSeconds) {
    // Thread-safe
    std::unique_lock<std::mutex> lock(this->task_mutex);
    this_thread::sleep_for(chrono::milliseconds(milliSeconds));
    this->taskQueue.emplace_back(task);
    this->cvNewTask.notify_one();
}

void Workers::start() {
    for(int i = 0; i < this->numberOfThreads; i++) {
        this->threadPool.emplace_back([&] {
            while(true) {//must be while true because taskQueue can be empty and afterwards recieve more tasks
                function<void> task;
                {//lock aquire
                    std::unique_lock<std::mutex> lock(this->task_mutex);
                    while(this->taskQueue.empty()) {
                        if(!running.load()) return;
                        this->cvNewTask.wait(lock);//unlock mutex and wait
                    }
                    if (!taskQueue.empty()) {
                        task = *taskQueue.begin();
                        taskQueue.pop_front();
                    }
                }//lock release
                this->cvNewTask.notify_one();
                if (task) {
                    task();//run task outside mutex lock
                }
            }
        });
    }
}

void Workers::stop() {
    std::unique_lock<std::mutex> lock(this->task_mutex);
    while(!this->taskQueue.empty()) {
        this->cvNewTask.wait(lock);
    }
    this->running.store(false);
    cvNewTask.notify_all();
}

void Workers::join() {
    std::thread t([&] {
        this->stop();
        for(auto& thread: this->threadPool) {
            thread.join();
        }
    });
    t.join();
}