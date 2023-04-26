#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void print_addrinfo (const struct addrinfo *pAddr)
{
    int ret = 0;

    printf ("begin of info:\n");
    if (NULL == pAddr)
    {
        printf ("null\n");
    }
    else
    {
        while (pAddr != NULL)
        {
            char szHost[256] = {};
            char szServ[256] = {};

            printf ("flags=%d,family=%d,socktype=%d,proto=%d,addlen=%d,addrp=%p,canoname=%s,next=%p\n",
                pAddr->ai_flags,
                pAddr->ai_family,
                pAddr->ai_socktype,
                pAddr->ai_protocol,
                pAddr->ai_addrlen,
                pAddr->ai_addr,
                pAddr->ai_canonname,
                pAddr->ai_next);

            ret = getnameinfo (pAddr->ai_addr, pAddr->ai_addrlen, 
                szHost, sizeof(szHost) - 1, szServ, sizeof(szServ) - 1, 
                NI_NUMERICHOST | NI_NUMERICSERV);
            if (ret)
            {
                printf ("get name info failed: ret=%d, info=%s\n", ret, gai_strerror(ret));
            }
            else
            {
                printf ("host=%s,serv=%s\n", szHost, szServ);
            }

            pAddr = pAddr->ai_next;
        }
    }
    printf ("end of info\n");
}

int main()
{
    while (1)
    {
        char *line_node = NULL;
        char *line_service = NULL;
        char *stmp = NULL;
        size_t linesize = 0;
        int ret = 0;
        struct addrinfo *addrinfo = NULL;
        struct addrinfo hints = {};

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_ALL | AI_PASSIVE | AI_CANONNAME;

        printf ("input node:\n");
        getline (& line_node, & linesize, stdin);
        printf ("input service:\n");
        getline (& line_service, & linesize, stdin);

        stmp = strstr (line_node, "\n");
        if (stmp != NULL)
        {
            *stmp = '\0';
        }
        stmp = strstr (line_service, "\n");
        if (stmp != NULL)
        {
            *stmp = '\0';
        }

        printf ("node=%s\n", line_node);
        printf ("service=%s\n", line_service);

        ret = getaddrinfo(line_node, line_service, & hints, &addrinfo);
        if (ret)
        {
            printf ("get failed, ret=%d, info=%s\n", ret, gai_strerror(ret));
        }
        else
        {
            print_addrinfo (addrinfo);
        }
        freeaddrinfo(addrinfo);

        free (line_node);
        free (line_service);
    }

    return 0;
}