#pragma once

#include <stddef.h>
#include <vector>
#include <string>
#include <stdint.h>

size_t enc_utf8_length(const std::string &utf8_in);
size_t enc_utf32_length(const std::vector<uint32_t> &utf32_in);

std::string enc_utf8_substr(const std::string &utf8_in, size_t pos, size_t count);
std::vector<uint32_t> enc_utf32_substr(const std::vector<uint32_t> &utf32_in, size_t pos, size_t count);

std::vector<uint32_t> enc_utf8_2_utf32(const std::string &utf8_in);
std::string enc_utf32_2_utf8(const std::vector<uint32_t> &utf32_in);