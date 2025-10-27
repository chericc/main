#pragma once

#include <memory>

class ExifTool {
public:
    ExifTool();
    ~ExifTool();

    bool parse(const char *filename, int timeoutMs);
private:
    bool init();
    bool sendCommand(const char *filename);
    bool readOutput(std::string &str, int timeoutMs);

    enum {
        READ_WAIT_MS = 500,
    };

    struct Ctx;
    std::shared_ptr<Ctx> _ctx = nullptr;
};