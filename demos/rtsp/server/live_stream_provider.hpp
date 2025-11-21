#pragma once

#include "xdemuxer.hpp"

#include <vector>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <list>

class LiveStreamProvider {
public:
    LiveStreamProvider() = default;
    virtual ~LiveStreamProvider() = default;

    enum Codec {
        None,
        H264,
        HEVC,
        AAC,
    };

    struct Info {
        bool hasVStream = false;
        bool hasAStream = false;
        Codec codecV = None;
        Codec codecA = None;
    };

    virtual bool info(Info&) = 0;
    virtual bool 
    virtual bool popVBuf(size_t size, std::vector<uint8_t> &buf) = 0;
    virtual bool popABuf(size_t size, std::vector<uint8_t> &buf) = 0;
};

class LiveStreamProviderFile : public LiveStreamProvider {
public:
    LiveStreamProviderFile(std::string file);
    virtual ~LiveStreamProviderFile();
    
    bool info(Info& info) override;
    bool popVBuf(size_t size, std::vector<uint8_t> &buf) override;
    bool popABuf(size_t size, std::vector<uint8_t> &buf) override;
private:

    enum {
        MAX_BUF_CACHE_NUM = 40,
    };

    bool popBuf(size_t size, std::vector<uint8_t> &buf, std::list<std::vector<uint8_t>> &pktList);
    bool tryPopBuf();

    struct Ctx;
    std::shared_ptr<Ctx> _ctx = nullptr;
};

