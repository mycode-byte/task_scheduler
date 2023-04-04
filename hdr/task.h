#ifndef __TASK_H__
#define __TASK_H__

#include <memory>

namespace afly{

template <class T>
class Task{
public:
    Task(){}
    virtual ~Task(){}

    virtual int process(std::shared_ptr<T> buff) = 0;

};  /*class Task*/

} /*namespace afly*/


#endif /*__TASK_H__*/