/**
 * @file netprint.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-01-22
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <netdb.h>

void np_ai_family_to_str(int ai_family, char* output, size_t size);

void np_ai_socktype_to_str(int ai_socktype, char* output, size_t size);

void np_ai_flags_to_str(int ai_flags, char* output, size_t size);

void np_ai_protocol_to_str(int ai_protocol, char* output, size_t size);

void np_addr_to_str(struct sockaddr* ai_addr, socklen_t ai_addrlen,
                    char* output, size_t size);

void np_addrinfo_to_str(const struct addrinfo* addrinfo, char* output,
                        size_t size);
