#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#include <stdio.h>

#include "netprint.h"

static void eg_getaddrinfo (const char *host, const char *service)
{
    int ret = 0;

    struct addrinfo *addrinfo = NULL;
    struct addrinfo addrhint = {};

    addrhint.ai_family = AF_UNSPEC;
    addrhint.ai_socktype = SOCK_DGRAM;
    addrhint.ai_flags = AI_PASSIVE;

    ret = getaddrinfo (host, service, & addrhint, & addrinfo);
    if (ret)
    {
        printf ("getaddrinfo failed, ret=%d, info=%s\n", ret, gai_strerror(ret));

        return ;
    }

    char output[512] = {};

    np_addrinfo_to_str(addrinfo, output, sizeof(output) - 1);
    printf ("output=%s\n", output);

    freeaddrinfo(addrinfo);
    addrinfo = NULL;

    return ;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf ("usage: %s [host] [service]\n", argv[0]);
        return 0;
    }

    eg_getaddrinfo(argv[1], argv[2]);

    return 0;
}