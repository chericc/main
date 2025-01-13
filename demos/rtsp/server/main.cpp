#include "xlog.hpp"

#include "rtsp_server.hpp"

int main()
{
    xlog_dbg("hello world\n");

    TaskScheduler *scheduler = nullptr;
    UsageEnvironment *env = nullptr;
    UserAuthenticationDatabase *authDB = nullptr;
    RTSPServer *rtspServer = nullptr;

    do {
        scheduler = BasicTaskScheduler::createNew();
        env = BasicUsageEnvironment::createNew(*scheduler);

        authDB = new UserAuthenticationDatabase;
        authDB->addUserRecord("admin", "123456");

        portNumBits port = 8888;
        rtspServer = MyRTSPServer::createNew(*env, port, authDB);
    
        if (!rtspServer) {
            xlog_err("create rtsp server failed\n");
            break;
        }

        env->taskScheduler().doEventLoop();
    } while (0);

    return 0;
}