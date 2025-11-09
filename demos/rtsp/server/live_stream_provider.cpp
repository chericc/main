#include "live_stream_provider.hpp"

#include <string>

struct LiveStreamProviderFile::Ctx {
    std::string file;
};

LiveStreamProviderFile::LiveStreamProviderFile(std::string file)
{
    _ctx = std::make_shared<Ctx>();

    _ctx->file = std::move(file);
}

LiveStreamProviderFile::~LiveStreamProviderFile()
{

}

bool LiveStreamProviderFile::popVBuf(size_t size, std::vector<uint8_t> &buf)
{
    return false;
}

bool LiveStreamProviderFile::popABuf(size_t size, std::vector<uint8_t> &buf)
{
    return false;
}