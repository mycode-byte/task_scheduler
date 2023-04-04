#include <iostream>
#include "task_scheduler.h"
#include <unistd.h>

using namespace afly;

struct frame{
    int index;
    int left;
    int right;
    int timestamp;
    frame():index(0),left(0),right(0),timestamp(0){}
};

class dewarp: public Task<frame>{
    virtual int process(std::shared_ptr<frame> buff){
        buff->index = 1;
        std::cout<<"process dewarp:"<<buff->index<<std::endl;
        return 0;
    }
};

class stereo: public Task<frame>{
    virtual int process(std::shared_ptr<frame> buff){
        buff->index = buff->index * 2;
        std::cout<<"process stereo:"<<buff->index<<std::endl;
        return 0;
    }
};

class od: public Task<frame>{
    virtual int process(std::shared_ptr<frame> buff){
        buff->index = buff->index * 2;
        std::cout<<"process od:"<<buff->index<<std::endl;
        return 0;
    }
};

class map: public Task<frame>{
    virtual int process(std::shared_ptr<frame> buff){
        buff->index = buff->index * 2;
        std::cout<<"process map:"<<buff->index<<std::endl;
        return 0;
    }
};


TaskScheduler<frame> g_ts;

//抓图
void capture_img(){
    while(true){
        std::shared_ptr<frame> f = std::make_shared<frame>();
        if(0 != g_ts.Submit(f)){
	    std::cout<<"sleep"<<std::endl;
            sleep(1);
        }
    }
}


int main()
{
    
    dewarp* p1 = new dewarp();
    stereo* p2 = new stereo();
    od* p3 = new od();
    map* p4 = new map();

    //set pipeline
    g_ts.AddTaskTemplate(p1);
    g_ts.AddTaskTemplate(p2);
    g_ts.AddTaskTemplate(p3);
    g_ts.AddTaskTemplate(p4);

    std::thread t(capture_img);

    //run
    g_ts.Run();

    t.join();

    delete p4;
    delete p3;
    delete p2;
    delete p1;
    
    return 0;
}
