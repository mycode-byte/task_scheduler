#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include "task.h"
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <future>
#include <functional>
#include <stdexcept>

namespace afly{

class ThreadPool{
public:
    ThreadPool(int thread_num=4);
    ~ThreadPool();

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;

private:
    std::vector<std::thread>            thread_pool_;
    std::queue<std::function<void()>>   tasks_;
    bool                                stop_;
    std::mutex                          tasks_mtx_;
    std::condition_variable             tasks_cond_;
};  /*class ThreadPool*/


ThreadPool::ThreadPool(int thread_num):stop_(false){
    for(int i=0; i<thread_num; ++i){
        thread_pool_.emplace_back([this]{
            while(true){
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->tasks_mtx_);
                    this->tasks_cond_.wait(lock, [this]{
                        return this->stop_ || !this->tasks_.empty();    //没任务时线程休眠
                    });

                    if(this->stop_ && this->tasks_.empty()){
                        return;
                    }

                    task = std::move(this->tasks_.front());
                    this->tasks_.pop();
                }

                task();
            }
        });
    }
}

ThreadPool::~ThreadPool(){
    {
        std::unique_lock<std::mutex> lock(this->tasks_mtx_);
        stop_ = true;
    }
    for(auto& thread : thread_pool_){
        thread.join();
    }
}

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>{

    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(tasks_mtx_);

        if(stop_){
            throw std::runtime_error("enqueue on stop ThreadPool");
        }

        tasks_.emplace([task](){
            (*task)();
        });
    }
    tasks_cond_.notify_one();
    return res;
}


} /*namespace afly*/


#endif /*__THREAD_POOL_H__*/
