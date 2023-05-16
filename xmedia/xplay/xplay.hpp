#pragma once

#include <string>
#include <thread>
#include <memory>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <vector>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/fifo.h>
}

struct OptValues
{
    std::string filename;
};


class PacketQueue
{
public:
    AVFifo *pkt_list{nullptr};
    int nb_packets{ 0 };
    int size{ 0 };
    int64_t duration{ 0 };
    int abort_request{ 0 };
    int serial{ 0 };
    std::mutex mutex;
    std::condition_variable cond;

    int init();
    int put(AVPacket* pkt);
    int putPrivate(AVPacket* pkt);
    int putNullPacket(AVPacket* pkt, int stream_index);
    void flush();
    void destroy();
    void abort();
    void start();
    int get(AVPacket* pkt, int block, int* serial);
};

/* Common struct for handling all types of decoded data and allocated render buffers. */
class Frame 
{
public:
    AVFrame *frame{nullptr};
    // AVSubtitle sub{};
    int serial{0};
    double pts{0.0};           /* presentation timestamp for the frame */
    double duration{0.0};      /* estimated duration of the frame */
    int64_t pos{0};          /* byte position of the frame in the input file */
    int width{0};
    int height{0};
    int format{0};
    AVRational sar{};
    int uploaded{0};
    int flip_v{0};
};

class FrameQueue
{
public:
    std::vector<Frame> queue;
    int rindex{0};
    int windex{0};
    int size{0};
    int max_size{0};
    int keep_last{0};
    int rindex_shown{0};
    std::mutex mutex;
    std::condition_variable cond;
    std::shared_ptr<PacketQueue> pktq;

    int init(std::shared_ptr<PacketQueue> pktq, int max_size);
    void destroy();
    void signal();
    Frame *peek();
    Frame *peek_next();
    Frame *peek_last();
    Frame *peek_writable();
    Frame *peek_readable();
    void push();
    void next();
    int numRemaining();
    int64_t lastPos();
};

class Decoder
{
public:
    AVPacket *pkt{nullptr};
    AVCodecContext *avctx{nullptr};
    PacketQueue *queue{nullptr};

    std::shared_ptr<std::thread> trd_decoder;

    int pkt_serial{};
    int finished{};
    int packet_pending{};
    std::condition_variable empty_queue_cond;
    int64_t start_pts{};
    AVRational start_pts_tb{};
    int64_t next_pts{};
    AVRational next_pts_tb{};
};

class VideoState
{
public:
    std::string filename;

    std::shared_ptr<std::thread> trd_read;

    AVFormatContext *ic{nullptr};

    int video_stream{0};
    AVStream *video_st{nullptr};

    std::shared_ptr<PacketQueue> videoq;

    std::shared_ptr<FrameQueue> pictq;

    Decoder auddec;
    Decoder viddec;

    int eof{};

    std::condition_variable cond_continue_read_thread;
};

class XWindow
{
public:
    XWindow();
};

class XPlay
{
public:
    XPlay();
    ~XPlay();
    int open(const OptValues &opt);
    int close();
private:
    int doOpen(const OptValues &opt);
    int doClose();
    int readThread();
    int videoThread();

    int streamComponentOpen(int stream_index);
    int decoderInit(Decoder *d, AVCodecContext *avctx, PacketQueue *queue);
    int decoderStart(Decoder *d, std::function<int()>, const char *thread_name);

    int getVideoFrame(AVFrame* frame);
    int decoderDecodeFrame(Decoder* d, AVFrame* frame, AVSubtitle* sub);

    std::shared_ptr<VideoState> streamOpen(const OptValues &opt);
    int streamClose(std::shared_ptr<VideoState> is);

    int queuePicture(AVFrame* src_frame, double pts, double duration, int64_t pos, int serial);

    int streamHasEnoughPackets(AVStream* st, int stream_id, PacketQueue* queue);
private:
    std::shared_ptr<VideoState> is_;

    std::mutex mutex_call_;
    using CallLock = std::lock_guard<std::mutex>;

    enum
    {
        MIN_FRAMES = 25,
        MAX_QUEUE_SIZE = 15 * 1024 * 1024,
    };
};
