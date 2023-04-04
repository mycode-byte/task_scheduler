#ifndef __TASK_SCHEDULER_H__
#define __TASK_SCHEDULER_H__

#include "task.h"
#include "thread_pool.h"
#include <list>
#include <vector>
#include <future>

namespace afly{

template <class T>
class TaskScheduler{
public:
        using task_ptr = typename std::list<Task<T>*>::iterator;
        struct task_info{
            std::shared_ptr<T>  buff;       //数据
            task_ptr            progress;   //进度
	        std::future<int>    result;     //结果
	        task_info(std::shared_ptr<T> b, task_ptr p):buff(b),progress(p){}
	        task_info(task_info& a):buff(a.buff),progress(a.progress){
		        result = std::move(a.result);
	        }
        };

    TaskScheduler(int task_num=2, int thread_num=4);
    ~TaskScheduler(){}

    //添加任务设置pipeline
    int AddTaskTemplate(Task<T>* t){
        if(nullptr == t){
            return -1;
        }
        pipeline_.push_back(t);
        return 0;
    }

    int Submit(std::shared_ptr<T> b);

    int Run();

    void Stop(){
        stop_=true;
    }

private:
    std::list<Task<T>*>             pipeline_;
    int                             task_num_;          //任务并发数

    std::mutex                      task_mtx_;          //任务锁
    int                             free_task_num_;     //空闲任务数
    std::list<std::shared_ptr<T>>   tasks_todo_;        //待执行的任务

    ThreadPool                      thread_pool_;       //线程池
    int                             stop_;
}; /*class TaskScheduler*/

template <class T>
TaskScheduler<T>::TaskScheduler(int task_num, int thread_num)
        :task_num_(task_num),
        free_task_num_(task_num),
        stop_(false){
}

template <class T>
int TaskScheduler<T>::Submit(std::shared_ptr<T> b){
    std::unique_lock<std::mutex> lock(task_mtx_);
    if(0 == free_task_num_){
        return -1;
    }
    tasks_todo_.push_back(b);
    return 0;
}

template <class T>
int TaskScheduler<T>::Run(){
        if(pipeline_.empty()){
            return -1;
        }

        //默认从pipeline第一个任务开始
        std::list<task_info>    tasks_ongoing_;      //执行中的任务

        while(!stop_){
            {
                //将待执行任务转换至执行中
                std::unique_lock<std::mutex> lock(task_mtx_);
                while(free_task_num_ && tasks_todo_.size()){
			        task_info ti(tasks_todo_.front(), pipeline_.begin());
                    tasks_todo_.pop_front();
                    --free_task_num_;
                    tasks_ongoing_.emplace_back(ti);
                }
            }

            //执行任务
            for(auto& ti : tasks_ongoing_){
                Task<T>* t = *ti.progress;
                std::shared_ptr<T> b = ti.buff;
                ti.result=std::move(thread_pool_.enqueue([t, b]{
                    return t->process(b);
                }));
            }

            //TODO 目前仅支持任务有序执行，有无序需求再添加
            for(auto iter = tasks_ongoing_.begin(); iter != tasks_ongoing_.end();){
                if(0 == iter->result.get()){
                    ++iter->progress;
                    if(iter->progress == pipeline_.end()){  //pipeline已经执行完毕
                        tasks_ongoing_.erase(iter++);
                        std::unique_lock<std::mutex> lock(task_mtx_);
                        ++free_task_num_;
                    }else{
                        ++iter;
                    }
                }else{      //任务失败则删除
                    tasks_ongoing_.erase(iter++);
                    std::unique_lock<std::mutex> lock(task_mtx_);
                    ++free_task_num_;
                }
            }

        }
        return 0;
    }


} /*namespace afly*/

#endif /*__TASK_SCHEDULER_H__*/
