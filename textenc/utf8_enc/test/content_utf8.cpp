#include "content_utf8.hpp"

std::string g_utf8_20_words = {(char)0xe4, (char)0xb8, (char)0x80, (char)0xe4,
                               (char)0xba, (char)0x8c, (char)0xe4,
                               (char)0xb8,  // 8
                               (char)0x89, (char)0xe5, (char)0x9b, (char)0x9b,
                               (char)0xe4, (char)0xba, (char)0x94,
                               (char)0xe5,  // 16
                               (char)0x85, (char)0xad, (char)0xe4, (char)0xb8,
                               (char)0x83, (char)0xe5, (char)0x85,
                               (char)0xab,  // 8
                               (char)0xe4, (char)0xb9, (char)0x9d, (char)0xe5,
                               (char)0x8d, (char)0x81, '1',
                               '2',  // 16
                               '3',        '4',        '5',        '6',
                               '7',        '8',        '9',        '0'};

std::string g_utf8_20_words_0_1 = {(char)0xe4, (char)0xb8, (char)0x80};

std::string g_utf8_20_words_0_0 = "";

std::string g_utf8_20_words_5_6 = {
    (char)0xe5, (char)0x85, (char)0xad, (char)0xe4,
    (char)0xb8, (char)0x83, (char)0xe5,
    (char)0x85,  // 8
    (char)0xab, (char)0xe4, (char)0xb9, (char)0x9d,
    (char)0xe5, (char)0x8d, (char)0x81,
    (char)0x31,  // 16
};

std::string g_utf8_20_words_19_1 = {'0'};

std::vector<uint32_t> g_utf32_20_words = {
    0x4E00, 0X4E8C, 0X4E09, 0X56DB, 0X4E94, 0X516D, 0X4E03,
    0X516B, 0X4E5D, 0X5341, '1',    '2',    '3',    '4',
    '5',    '6',    '7',    '8',    '9',    '0'};
