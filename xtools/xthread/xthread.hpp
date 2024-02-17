
/**
 * xthread
 * 
 * 20230603
 * 
 * An wrapper of std::thread
*/

#pragma once

#include <memory>
#include <functional>
#include <thread>

#include "xclass.hpp"

class XThread : public XNonCopyableObject
{
public:
    using _XFunc=std::function<void(void)>;
    XThread(_XFunc func);
    ~XThread();
    void start();
    void join();
private:
    std::shared_ptr<std::thread> _trd;
    _XFunc _func;
};
