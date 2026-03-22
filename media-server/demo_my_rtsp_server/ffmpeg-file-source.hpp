#pragma once

#include <memory>
#include "media-source.hpp"

class FFFileSource : public IMediaSource
{
public:
    FFFileSource(const char *file);
    virtual ~FFFileSource();

    int Play() override;
	int Pause() override;
	int Seek(int64_t pos) override;
	int SetSpeed(double speed) override;
	int GetDuration(int64_t& duration) const override;
    int GetSDPMedia(std::string& sdp) const override;
	int GetRTPInfo(const char* uri, char *rtpinfo, size_t bytes) const override;
	int SetTransport(const char* track, std::shared_ptr<IRTPTransport> transport) override;

private:
	struct Context;
	std::shared_ptr<Context> _ctx = nullptr;
};
