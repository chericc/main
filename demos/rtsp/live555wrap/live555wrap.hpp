#pragma once

#include <string>
#include <map>

#include "BasicUsageEnvironment.hh"

using std::map;
using std::string;

class LW_Env : public BasicUsageEnvironment
{
public:
    LW_Env(char &stop);
    int loop();
protected:
    char &_stop;
};

class LW_SessionCB
{
public:
    virtual int onNewSesion();
};

class LW_RTSPConnection
{
public:

    static int decodeTimeoutOption(const map<string,string> &opts);
    static int decodeRTPTransport(const map<string,string> &opts);

    // LW_RTSPConnection(LW_Env &env, )
};