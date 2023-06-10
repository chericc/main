#pragma once

#include "depends.hpp"

#include "xthread.hpp"
#include "packetqueue.hpp"
#include "framequeue.hpp"

struct OptValues
{
    std::string filename;
};

class Decoder
{
public:
    AVPacket *pkt{nullptr};
    AVCodecContext *avctx{nullptr};
    PacketQueue *queue{nullptr};

    std::shared_ptr<XThread> trd_decoder;

    int pkt_serial{};
    int finished{};
    int packet_pending{};
    std::shared_ptr<std::condition_variable> empty_queue_cond;
    int64_t start_pts{};
    AVRational start_pts_tb{};
    int64_t next_pts{};
    AVRational next_pts_tb{};

    int init(AVCodecContext *avctx, PacketQueue *queue, std::shared_ptr<std::condition_variable> empty_queue_cond);
    int start(std::function<int()> func, const char *thread_name);
    int decodeFrame(AVFrame* frame, AVSubtitle* sub);
};

class VideoState
{
public:
    std::string filename;

    std::shared_ptr<XThread> trd_read;

    AVFormatContext *ic{nullptr};

    int video_stream{0};
    AVStream *video_st{nullptr};

    std::shared_ptr<PacketQueue> videoq;

    std::shared_ptr<FrameQueue> pictq;

    Decoder auddec;
    Decoder viddec;

    int eof{};

    std::shared_ptr<std::condition_variable> cond_continue_read_thread;

    double max_frame_duration{ 0.0 };

    struct RefreshCtx
    {
        double frame_timer{0.0};
        bool force_refresh{false};
    } refreshctx;
};

struct RefreshState
{
    const Frame *frame{nullptr};
    double remaining_time{0.0};
};

class XPlay
{
public:
    XPlay();
    ~XPlay();
    int open(const OptValues &opt);
    int close();

    int refresh(RefreshState *state);
private:
    int doOpen(const OptValues &opt);
    int doClose();
    int readThread();
    int videoThread();

    int streamComponentOpen(int stream_index);

    int getVideoFrame(AVFrame* frame);

    std::shared_ptr<VideoState> streamOpen(const OptValues &opt);
    int streamClose(std::shared_ptr<VideoState> is);

    int queuePicture(AVFrame* src_frame, double pts, double duration, int64_t pos, int serial);

    int streamHasEnoughPackets(AVStream* st, int stream_id, PacketQueue* queue);
private:
    int doRefresh(RefreshState *state);
    double vp_duration(Frame *vp, Frame *nextvp);
    double compute_target_delay(double delay);
private:
    std::shared_ptr<VideoState> _is;

    std::mutex mutex_call_;
    using CallLock = std::lock_guard<std::mutex>;

    enum
    {
        MIN_FRAMES = 25,
        MAX_QUEUE_SIZE = 15 * 1024 * 1024,
    };
    const double AV_SYNC_THRESHOLD_MAX{0.1};
};
