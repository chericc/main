#pragma once

#include <string>
#include <array>

std::pair<std::string,std::string> str_ipv4addr(const std::array<uint8_t, 8> &ipv4);
std::string str_ipv6addr(const std::array<uint8_t, 17> &ipv6);
std::string str_macaddr(const std::array<uint8_t, 6> &mac);
std::string str_euiaddr(const std::array<uint8_t, 8> &euiaddr);
std::string str_bps(double bps);