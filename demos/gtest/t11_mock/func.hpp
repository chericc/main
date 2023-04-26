#pragma once

#include <memory>
#include "dep.hpp"

class Func
{
public:
    Func(std::shared_ptr<UploaderInterface> uploader, int ntimes);
    int do_sth();
protected:
    std::shared_ptr<UploaderInterface> _uploader;
    int _ntimes{0};
};