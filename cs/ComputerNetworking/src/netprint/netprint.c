#include "netprint.h"

#include <sys/socket.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void np_ai_family_to_str (int ai_family, char *output, size_t size)
{
    const char *name = NULL;

    switch (ai_family)
    {
        case AF_UNSPEC:
        {
            name = "AF_UNSPEC";
            break;
        }
        case AF_INET:
        {
            name = "AF_INET";
            break;
        }
        case AF_INET6:
        {
            name = "AF_INET6";
            break;
        }
        default:
        {
            break;
        }
    }

    if (NULL != name)
    {
        snprintf (output, size, "%s", name);
    }
    else
    {
        snprintf (output, size, "ai_family=%#x unsupport\n", ai_family);
    }

    return ;
}

void np_ai_socktype_to_str (int ai_socktype, char *output, size_t size)
{
    const char *name = NULL;

    switch (ai_socktype)
    {
        case SOCK_STREAM:
        {
            name = "SOCK_STREAM";
            break;
        }
        case SOCK_DGRAM:
        {
            name = "SOCK_DGRAM";
            break;
        }
        default:
        {
            break;
        }
    }

    if (NULL != name)
    {
        snprintf (output, size, "%s", name);
    }
    else
    {
        snprintf (output, size, "ai_socktype=%#x unsupport\n", ai_socktype);
    }

    return ;
}

void np_ai_flags_to_str (int ai_flags, char *output, size_t size)
{
    int bFound = false;
    int nOutputPos = 0;
    char tmpbuffer[256] = {};       // need be big enough
    int nStrLen = 0;

    int arrayValue[] = {
        AI_PASSIVE,
        AI_CANONNAME,
        AI_NUMERICHOST,
        AI_V4MAPPED,
        AI_ALL,
        AI_ADDRCONFIG,
        AI_NUMERICSERV,
    };

    const char *arrayNames[] = {
        "AI_PASSIVE",
        "AI_CANONNAME",
        "AI_NUMERICHOST",
        "AI_V4MAPPED",
        "AI_ALL",
        "AI_ADDRCONFIG",
        "AI_NUMERICSERV",
    };

    const int VALUE_ITEM_NUM = sizeof(arrayValue) / sizeof(arrayValue[0]);
    const int NAMES_ITEM_NUMVER = sizeof(arrayNames) / sizeof(arrayNames[0]);
    const int MIN_ITEMS_NUM = (VALUE_ITEM_NUM < NAMES_ITEM_NUMVER ? VALUE_ITEM_NUM : NAMES_ITEM_NUMVER);

    for (size_t index = 0; index < MIN_ITEMS_NUM; ++index)
    {
        if (ai_flags & arrayValue[index])
        {
            bFound = true;
            nOutputPos += sprintf (tmpbuffer + nOutputPos, "%s,", arrayNames[index]);
        }
    }

    if (bFound)
    {
        nStrLen = strlen (tmpbuffer);

        if (nStrLen > 0)
        {
            tmpbuffer[nStrLen - 1] = '\0'; // replace the last ','
        }

        strncpy (output, tmpbuffer, size);
    }
    else
    {
        snprintf (output, size, "ai_flag=%#x not support", ai_flags);
    }

    return ;
}

void np_ai_protocol_to_str (int ai_protocol, char *output, size_t size)
{
    int bFound = false;

    int arrayValues[] = {
        IPPROTO_IP,
        IPPROTO_ICMP,
        IPPROTO_IGMP,
        IPPROTO_IPIP,
        IPPROTO_TCP,
        IPPROTO_EGP,
        IPPROTO_PUP,
        IPPROTO_UDP,
        IPPROTO_IDP,
        IPPROTO_TP,
        IPPROTO_DCCP,
        IPPROTO_IPV6,
        IPPROTO_RSVP,
        IPPROTO_GRE,
        IPPROTO_ESP,
        IPPROTO_AH,
        IPPROTO_MTP,
        IPPROTO_BEETPH,
        IPPROTO_ENCAP,
        IPPROTO_PIM,
        IPPROTO_COMP,
        IPPROTO_SCTP,
        IPPROTO_UDPLITE,
        IPPROTO_MPLS,
        IPPROTO_RAW,
    };

    const char *arrayNames[] = {
        "IPPROTO_IP",
        "IPPROTO_ICMP",
        "IPPROTO_IGMP",
        "IPPROTO_IPIP",
        "IPPROTO_TCP",
        "IPPROTO_EGP",
        "IPPROTO_PUP",
        "IPPROTO_UDP",
        "IPPROTO_IDP",
        "IPPROTO_TP",
        "IPPROTO_DCCP",
        "IPPROTO_IPV6",
        "IPPROTO_RSVP",
        "IPPROTO_GRE",
        "IPPROTO_ESP",
        "IPPROTO_AH",
        "IPPROTO_MTP",
        "IPPROTO_BEETPH",
        "IPPROTO_ENCAP",
        "IPPROTO_PIM",
        "IPPROTO_COMP",
        "IPPROTO_SCTP",
        "IPPROTO_UDPLITE",
        "IPPROTO_MPLS",
        "IPPROTO_RAW",
    };

    const int VALUE_ITEM_NUM = sizeof(arrayValues) / sizeof(arrayValues[0]);
    const int NAMES_ITEM_NUMVER = sizeof(arrayNames) / sizeof(arrayNames[0]);
    const int MIN_ITEMS_NUM = (VALUE_ITEM_NUM < NAMES_ITEM_NUMVER ? VALUE_ITEM_NUM : NAMES_ITEM_NUMVER);

    for (size_t index = 0; index < MIN_ITEMS_NUM; ++index)
    {
        if (arrayValues[index] == ai_protocol)
        {
            bFound = true;
            snprintf (output, size, "%s", arrayNames[index]);
        }
    }

    if (! bFound)
    {
        snprintf (output, size, "ai_protocol=%#x unsupport", ai_protocol);
    }

    return ;
}

void np_addr_to_str(struct sockaddr *ai_addr, socklen_t ai_addrlen, char *output, size_t size)
{
    char host[NI_MAXHOST] = {};
    char service[NI_MAXHOST] = {};

    int ret = 0;

    ret = getnameinfo (ai_addr, ai_addrlen, host, sizeof(host), service, sizeof(service), 
            NI_NUMERICHOST | NI_NUMERICSERV);
    
    if (0 == ret)
    {
        snprintf (output, size, "host: %s service: %s", host, service);
    }
    else
    {
        printf ("getnameinfo failed: %s\n", gai_strerror(ret));
        snprintf (output, size, "fail");
    }
    
    return ;
}

void np_addrinfo_to_str (const struct addrinfo *addrinfo, char *output, size_t size)
{
    char sflag[64];
    char sfamily[64];
    char ssocktype[64];
    char sprotocol[64];
    char saddr[64];

    int nOutPos = 0;

    while (NULL != addrinfo)
    {
        np_ai_flags_to_str(addrinfo->ai_flags, sflag, sizeof(sflag) - 1);
        np_ai_family_to_str(addrinfo->ai_family, sfamily, sizeof(sfamily) - 1);
        np_ai_socktype_to_str(addrinfo->ai_socktype, ssocktype, sizeof(ssocktype) - 1);
        np_ai_protocol_to_str(addrinfo->ai_protocol, sprotocol, sizeof(sprotocol) - 1);
        np_addr_to_str(addrinfo->ai_addr, addrinfo->ai_addrlen, saddr, sizeof(saddr) - 1);

        if (nOutPos < size)
        {
            nOutPos += snprintf (output + nOutPos, size - nOutPos, "[flag=%s,family=%s,socktype=%s,protocol=%s,addr=(%s),cononname=%s]",
                                    sflag, sfamily, ssocktype, sprotocol, saddr, addrinfo->ai_canonname);
        }

        addrinfo  = addrinfo->ai_next;
    }
}