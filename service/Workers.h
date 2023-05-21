#ifndef MESH_NETWORK_WORKERS_H
#define MESH_NETWORK_WORKERS_H
#include <functional>
#include <iostream>
#include <list>
#include <vector>
#include <thread>
#include <condition_variable>

class Workers{
    int numberOfThreads;
    std::list<function<void>> taskQueue;
    std::vector<std::thread> threadPool;
    std::condition_variable cvNewTask;
    std::mutex task_mutex;
    atomic<bool> running{true};

public:
    explicit Workers(int numberOfThreads){
        this->numberOfThreads = numberOfThreads;
    };

    void post(const std::function<void()>&);
    void post_timeout(const std::function<void()>&, int);
    void start();
    void stop();
    void join();
};
#endif //MESH_NETWORK_WORKERS_H