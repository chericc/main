#pragma once

#include <stdint.h>

#include <array>
#include <string>

enum ENUMS__ {
    MD5_LEN = 16,  // MD5 size is 16, don't change it
};

class FileItem {
   public:
    std::string file_path;
    bool operator<(const FileItem& ref) const {
        return file_path < ref.file_path;
    }
};

class Md5Result {
   public:
    int worker_id{0};
    std::string file_path;
    std::array<uint8_t, MD5_LEN> md5;
    bool operator<(const Md5Result& ref) const {
        return file_path < ref.file_path;
    }
};