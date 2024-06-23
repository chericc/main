#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

size_t fileSize(FILE* fp);

std::vector<uint16_t> readFile16(const std::string& filename);
std::vector<uint8_t> readFile8(const std::string& filename);
std::vector<uint8_t> readFile(const std::string& filename);

void saveFile(const std::string& filename, std::vector<uint8_t> data);