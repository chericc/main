#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int PrepareUDPSocketIPv4Server (const char *szHost, const char *szService)
{
    int ret = 0;
    struct addrinfo *addrinfo = NULL;
    struct addrinfo addrhints = {};
    struct addrinfo *addrinfo_iterator = NULL;
    char szHostTmp[64] = {};
    char szServiceTmp[64] = {};
    int nSocket = -1;
    int bError = false;

    strncpy (szHostTmp, szHost, sizeof(szHostTmp) - 1);
    strncpy (szServiceTmp, szService, sizeof(szServiceTmp) - 1);

    addrhints.ai_family = AF_INET;
    addrhints.ai_socktype = SOCK_DGRAM;
    addrhints.ai_flags = AI_PASSIVE;

    do 
    {
        ret = getaddrinfo (szHostTmp, szServiceTmp, & addrhints, & addrinfo);
        if (ret)
        {
            printf ("getaddrinfo failed, ret=%d, info=%s\n", ret, gai_strerror(ret));
            bError = true;
            break;
        }

        for (addrinfo_iterator = addrinfo; 
                addrinfo_iterator != NULL; 
                addrinfo_iterator = addrinfo_iterator->ai_next)
        {
            nSocket = socket (addrinfo_iterator->ai_family, addrinfo_iterator->ai_socktype, addrinfo_iterator->ai_protocol);
            if (nSocket < 0)
            {
                continue ;
            }

            if (0 == bind (nSocket, addrinfo_iterator->ai_addr, addrinfo_iterator->ai_addrlen))
            {
                break;
            }

            close (nSocket);
        }

        if (NULL == addrinfo_iterator)
        {
            printf ("socket or bind failed\n");
            bError = true;
            break;
        }
    }
    while (0);

    if (NULL != addrinfo)
    {
        freeaddrinfo(addrinfo);
        addrinfo = NULL;
    }

    if (bError)
    {
        printf ("error happend\n");
        if (nSocket >= 0)
        {
            printf ("releasing socket\n");
            close (nSocket);
            nSocket = -1;
        }
    }

    return bError ? -1 : nSocket;
}

void UdpServer (const char *szHost, const char *szService)
{
    int ret = 0;
    char buffer[256] = {};
    ssize_t ssize_ret = 0;

    int fd = PrepareUDPSocketIPv4Server(szHost, szService);
    if (fd < 0)
    {
        printf ("udp server failed\n");
        return ;
    }

    printf ("udp server successful\n");

    while (true)
    {
        struct sockaddr_storage sSockAddrStorage = {};
        socklen_t nSockLen = 0;

        memset (buffer, 0, sizeof(buffer));

        nSockLen = sizeof(sSockAddrStorage);
        ssize_ret = recvfrom (fd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)(&sSockAddrStorage), & nSockLen);
        if (ssize_ret < 0)
        {
            perror ("recvfrom failed");
            continue ;
        }

        printf ("%s", buffer);

        ssize_ret = sendto (fd, buffer, ssize_ret, 0, (struct sockaddr*)(&sSockAddrStorage), nSockLen);
        if (ssize_ret < 0)
        {
            perror ("recvfrom failed");
            continue ;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf ("usage: %s [host] [service(port)]\n", argv[0]);
        return 0;
    }

    UdpServer(argv[1], argv[2]);

    return 0;
}