#include <xlog.h>

#include "global_config.hpp"

int main() {
    xlog_dbg("name: {}, sex: {}",
             GlobalConfig::getInstance().valueOf("name").c_str(),
             GlobalConfig::getInstance().valueOf("sex").c_str());
    return 0;
}