#include "xthread.hpp"

XThread::XThread(_XFunc func) { _func = func; }

XThread::~XThread() { join(); }

void XThread::start() { _trd = std::make_shared<std::thread>(_func); }

void XThread::join() {
    if (_trd && _trd->joinable()) {
        _trd->join();
    }
}
