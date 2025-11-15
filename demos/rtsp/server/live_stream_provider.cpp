#include "live_stream_provider.hpp"

#include <string>
#include <memory>

#include "xdemuxer.hpp"
#include "xlog.h"

struct LiveStreamProviderFile::Ctx {
    std::string file;

    std::shared_ptr<XDemuxer> demuxer = nullptr;
};

LiveStreamProviderFile::LiveStreamProviderFile(std::string file)
{
    _ctx = std::make_shared<Ctx>();

    _ctx->file = std::move(file);

    auto demuxer = std::make_shared<XDemuxer>(file);
    if (demuxer->open()) {
        _ctx->demuxer = demuxer;
    } else {
        xlog_err("open failed\n");
    }
}

LiveStreamProviderFile::~LiveStreamProviderFile()
{

}

bool LiveStreamProviderFile::info(Info &info)
{
    bool okFlag = false;
    do {
        if (nullptr == _ctx->demuxer) {
            break;
        }

        // info.
    } while (false);
}

bool LiveStreamProviderFile::popVBuf(size_t size, std::vector<uint8_t> &buf)
{
    return false;
}

bool LiveStreamProviderFile::popABuf(size_t size, std::vector<uint8_t> &buf)
{
    return false;
}