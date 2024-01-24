#ifndef TSPARSER_HPP__
#define TSPARSER_HPP__

#include <string>

class TsInfo
{

};

class TsParserTasks
{
public:
    bool get_ts_info = false;
    bool get_sei = false;
    bool get_es = false;
};

class TsParser
{
public:
    TsParser();
    ~TsParser();

    /**
     * 设置parser的任务；
     * 若这些任务均有数据可以获取，writeData时
     * 就能查询到OK；
    */
    int setTasks(const TsParserTasks &tasks);

    /**
     * 写入ts数据
    */
    int writeData(void *data, std::size_t size);

    /**
     * 设置对象的身份信息（可以用来打印日志）
    */
    void setId(const std::string &id);

    /**
     * 
    */
};

#endif // TSPARSER_HPP__