#pragma once

#include <array>
#include <string>

std::string str_ipv4(const std::array<uint8_t, 4>& ipv4);
std::string str_ipv6(const std::array<uint8_t, 16>& ipv6);
std::pair<std::string, std::string> str_ipv4addr(
    const std::array<uint8_t, 8>& ipv4);
std::string str_ipv6addr(const std::array<uint8_t, 17>& ipv6);
std::string str_macaddr(const std::array<uint8_t, 6>& mac);
std::string str_euiaddr(const std::array<uint8_t, 8>& euiaddr);
std::string str_bps(double bps);

uint8_t net_u8(const void* ptr, std::size_t size);
uint16_t net_u16(const void* ptr, std::size_t size);
uint32_t net_u32(const void* ptr, std::size_t size);
uint64_t net_u64(const void* ptr, std::size_t size);
std::size_t net_copy(void* dst, std::size_t dst_size, void* src,
                     std::size_t src_size);
