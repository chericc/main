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

void UdpClient(const char *szHost, const char *szService)
{
    int ret = 0;
    char buffer[256] = {};
    ssize_t ssize_ret = 0;
    int nSocket = -1;
    int bError = false;
    int nLen = 0;
    char *input = NULL;
    size_t size_input = 0;

    struct addrinfo *addrinfo = NULL;
    struct addrinfo addrhints = {};

    addrhints.ai_family = AF_INET;
    addrhints.ai_socktype = SOCK_DGRAM;
    addrhints.ai_flags = 0;

    do 
    {
        ret = getaddrinfo (szHost, szService, & addrhints, & addrinfo);
        if (ret)
        {
            bError = true;
            printf ("getaddrinfo failed, ret=%d, info=%s\n", ret, gai_strerror(ret));
            break;
        }

        if (NULL != addrinfo)
        {
            nSocket = socket (addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);
        }
        else
        {
            printf ("null addrinfo\n");
            bError = true;
        }
    }
    while (false);

    if (bError)
    {
        if (nSocket > 0)
        {
            close (nSocket);
            nSocket = -1;
        }
    }
    else
    {
        printf ("udp client successful\n");
    }


    if (!bError)
    {
        while (true)
        {
            struct sockaddr_storage sSockAddrStorage = {};
            socklen_t nSockLen = 0;
            
            getline (& input, & size_input, stdin);

            ssize_ret = sendto (nSocket, input, size_input, 0, addrinfo->ai_addr, addrinfo->ai_addrlen);

            free (input);
            input = NULL;

            if (ssize_ret < 0)
            {
                perror ("sendto failed");
                continue ;
            }

            printf ("%s\n", buffer);
        }
    }

    if (NULL != addrinfo)
    {
        freeaddrinfo (addrinfo);
        addrinfo = NULL;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf ("usage: %s [server] [service(port)]\n", argv[0]);
        return 0;
    }

    UdpClient(argv[1], argv[2]);

    return 0;
}