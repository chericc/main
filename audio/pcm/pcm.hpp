#pragma once

#include <stdint.h>

#include <vector>

std::vector<uint8_t> pcm_s16le_to_alaw(const std::vector<uint16_t>& pcm_s16le);
std::vector<uint16_t> pcm_alaw_to_s16e(const std::vector<uint8_t>& pcm_alaw);

std::vector<uint8_t> pcm_s16le_to_mulaw(const std::vector<uint16_t>& pcm_s16le);
std::vector<uint16_t> pcm_mulaw_to_s16e(const std::vector<uint8_t>& pcm_mulaw);
