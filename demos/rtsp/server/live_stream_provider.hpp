#pragma once

#include "xdemuxer.hpp"

#include <vector>
#include <cstdint>
#include <cstddef>
#include <memory>

class LiveStreamProvider {
public:
    LiveStreamProvider() = default;
    virtual ~LiveStreamProvider() = default;

    struct Info {
        bool hasVStream;
        bool hasAStream;
        
    };

    virtual bool popVBuf(size_t size, std::vector<uint8_t> &buf) = 0;
    virtual bool popABuf(size_t size, std::vector<uint8_t> &buf) = 0;
};

class LiveStreamProviderFile : public LiveStreamProvider {
public:
    LiveStreamProviderFile(std::string file);
    virtual ~LiveStreamProviderFile();
    
    bool popVBuf(size_t size, std::vector<uint8_t> &buf) override;
    bool popABuf(size_t size, std::vector<uint8_t> &buf) override;
private:
    struct Ctx;
    std::shared_ptr<Ctx> _ctx = nullptr;
};

