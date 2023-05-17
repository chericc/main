

#include "xplay.hpp"
#include "xlog.hpp"

#include <assert.h>

#define array_sizeof(array) ((sizeof(array))/(sizeof(array[0])))

typedef struct MyAVPacketList {
    AVPacket* pkt{nullptr};
    int serial{ 0 };
} MyAVPacketList;

MyThread::MyThread(MyFunc func_tmp)
{
    func = func_tmp;
}

MyThread::~MyThread()
{
    std::unique_lock<std::mutex> lock(mutex);
    if (trd && trd->joinable())
    {
        trd->join();
    }
}

void MyThread::start()
{
    std::unique_lock<std::mutex> lock(mutex);
    if (!trd)
    {
        trd = std::make_shared<std::thread>(func);
    }
}

void MyThread::join()
{
    std::unique_lock<std::mutex> lock(mutex);
    if (trd && trd->joinable())
    {
        trd->join();
    }
}

int FrameQueue::init(std::shared_ptr<PacketQueue> pktq_v, int max_size)
{
    pktq = pktq_v;
    queue.resize(max_size);
    for (auto &r : queue)
    {
        r.frame = av_frame_alloc();
        if (!r.frame)
        {
            xlog_err("alloc failed");
            return -1;
        }
    }
    return 0;
}

void FrameQueue::destroy()
{
    for (auto &r : queue)
    {
        if (r.frame)
        {
            av_frame_unref(r.frame);
            av_frame_free(&r.frame);
        }
    }
}

void FrameQueue::signal()
{
    std::unique_lock<std::mutex> lock(mutex);
    cond.notify_one();
}

Frame *FrameQueue::peek()
{
    return &queue[(rindex + rindex_shown) % queue.size()];
}

Frame *FrameQueue::peek_next()
{
    return &queue[(rindex + rindex_shown + 1) % queue.size()];
}

Frame *FrameQueue::peek_last()
{
    return &queue[rindex];
}

Frame *FrameQueue::peek_writable()
{
    std::unique_lock lock(mutex);
    while (size >= queue.size()
        && !pktq->abort_request)
    {
        cond.wait(lock);
    }

    if (pktq->abort_request)
    {
        return nullptr;
    }

    return &queue[windex];
}

Frame *FrameQueue::peek_readable()
{
    std::unique_lock<std::mutex> lock(mutex);
    while (size - rindex_shown <= 0
        && !pktq->abort_request)
    {
        cond.wait(lock);
    }

    if (pktq->abort_request)
    {
        return nullptr;
    }
    return &queue[(rindex + rindex_shown) % queue.size()];
}

void FrameQueue::push()
{
    if (++windex == queue.size())
    {
        windex = 0;
    }
    std::unique_lock<std::mutex> lock(mutex);
    ++size;
    cond.notify_one();
}

void FrameQueue::next()
{
    if (keep_last && !rindex_shown)
    {
        rindex_shown = 1;
        return ;
    }

    if (queue[rindex].frame)
    {
        av_frame_unref(queue[rindex].frame);
        av_frame_free(&queue[rindex].frame);
    }

    std::unique_lock<std::mutex> lock(mutex);
    --size;
    cond.notify_one();
}

int FrameQueue::numRemaining()
{
    return size - rindex_shown;
}

int64_t FrameQueue::lastPos()
{
    Frame *fp = &queue[rindex];
    if (rindex_shown && fp->serial == pktq->serial)
    {
        return fp->pos;
    }
    else 
    {
        return -1;
    }
}

int PacketQueue::get(AVPacket* pkt, int block, int* serial)
{
    std::unique_lock<std::mutex> lock(mutex);

    MyAVPacketList pkt1;
    int ret = 0;

    for (;;)
    {
        if (abort_request)
        {
            ret = -1;
            break;
        }

        if (av_fifo_read(pkt_list, &pkt1, 1) >= 0)
        {
            nb_packets--;
            size -= pkt1.pkt->size + sizeof(pkt1);
            duration -= pkt1.pkt->duration;
            av_packet_move_ref(pkt, pkt1.pkt);
            if (serial)
            {
                *serial = pkt1.serial;
            }
            av_packet_free(&pkt1.pkt);
            ret = 1;
            
            xlog_trc("packet queue.nb=%d", nb_packets);

            break;
        }
        else if (!block)
        {
            ret = 0;
            break;
        }
        else
        {
            cond.wait(lock);
        }
    }
    return ret;
}

void PacketQueue::start()
{
    std::unique_lock<std::mutex> lock;
    abort_request = 0;
    serial++;
    return;
}

void PacketQueue::abort()
{
    std::unique_lock<std::mutex> lock;
    abort_request = 1;
    cond.notify_one();
    return;
}

void PacketQueue::destroy()
{
    flush();
    av_fifo_freep2(&pkt_list);
    return;
}

void PacketQueue::flush()
{
    std::unique_lock<std::mutex> lock;
    MyAVPacketList pkt1;
    
    while (av_fifo_read(pkt_list, &pkt1, 1) >= 0)
    {
        av_packet_free(&pkt1.pkt);
    }
    nb_packets = 0;
    size = 0;
    duration = 0;
    serial++;
    return;
}

int PacketQueue::init()
{
    pkt_list = av_fifo_alloc2(1, sizeof(MyAVPacketList), AV_FIFO_FLAG_AUTO_GROW);
    if (!pkt_list)
    {
        return AVERROR(ENOMEM);
    }
    abort_request = 1;
    return 0;
}

int PacketQueue::putNullPacket(AVPacket* pkt, int stream_index)
{
    pkt->stream_index = stream_index;
    return put(pkt);
}

int PacketQueue::put(AVPacket* pkt)
{
    AVPacket* pkt1{};
    int ret{};

    pkt1 = av_packet_alloc();
    if (!pkt1)
    {
        av_packet_unref(pkt);
        return -1;
    }
    av_packet_move_ref(pkt1, pkt);

    std::unique_lock<std::mutex> lock(mutex);
    ret = putPrivate(pkt1);
    lock.unlock();

    if (ret < 0)
    {
        av_packet_free(&pkt1);
    }
    return ret;
}

int PacketQueue::putPrivate(AVPacket* pkt)
{
    MyAVPacketList pkt1{};
    int ret{};

    if (abort_request)
    {
        return -1;
    }

    pkt1.pkt = pkt;
    pkt1.serial = serial;

    ret = av_fifo_write(pkt_list, &pkt1, 1);
    if (ret < 0)
    {
        return ret;
    }
    nb_packets++;
    size += pkt1.pkt->size + sizeof(pkt1);
    duration += pkt1.pkt->duration;
    cond.notify_one();

    xlog_trc("packet queue.nb=%d", nb_packets);

    return 0;
}

XPlay::XPlay()
{

}

XPlay::~XPlay()
{
    doClose();
}

int XPlay::open(const OptValues &opt)
{
    CallLock calllock(mutex_call_);
    
    int ret = 0;

    do 
    {
        if (is_)
        {
            xlog_err("already opened");
            ret = -1;
            break;
        }

        ret = doOpen(opt);
    }
    while (0);

    return ret;
}

int XPlay::close()
{
    CallLock calllock(mutex_call_);
    int ret = 0;

    if (!is_)
    {
        xlog_err("not opened");
    }

    ret = doClose();

    return ret;
}

int XPlay::doOpen(const OptValues &opt)
{
    int ret = 0;

    if (is_)
    {
        doClose();
    }

    assert(nullptr == is_);

    is_ = streamOpen(opt);

    /* 赋值之后再启动线程 */
    if (is_ && is_->trd_read)
    {
        is_->trd_read->start();
    }
    
    return is_ ? 0 : -1;
}

int XPlay::doClose()
{
    int ret = 0;

    if (is_)
    {
        if (streamClose(is_) < 0)
        {
            xlog_err("close failed");
            ret = -1;
        }
    }

    is_.reset();

    return ret;
}

std::shared_ptr<VideoState> XPlay::streamOpen(const OptValues &opt)
{
    std::shared_ptr<VideoState> is;

    xlog_trc("open:[file=%s]", opt.filename.c_str());

    do
    {
        is = std::make_shared<VideoState>();

        is->filename = opt.filename;

        is->videoq = std::make_shared<PacketQueue>();
        is->pictq = std::make_shared<FrameQueue>();

        if (is->pictq->init(is->videoq, 16) < 0)
        {
            xlog_err("pic queue init failed");
            break;
        }

        if (is->videoq->init() < 0)
        {
            xlog_err("video q init failed");
            break;
        }

        xlog_trc("videoq init suc");

        is->cond_continue_read_thread = std::make_shared<std::condition_variable>();

        auto lambda_readtrd = [&](){this->readThread();};
        is->trd_read = std::make_shared<MyThread>(lambda_readtrd);
    }
    while (0);

    return is;
}

int XPlay::streamComponentOpen(int stream_index)
{
    xlog_trc("streamComponentOpen index=%d", stream_index);

    int berror = false;

    AVCodecContext *avctx = nullptr;
    AVFormatContext *ic = nullptr;
    const AVCodec *codec = nullptr;

    int ret = 0;

    do
    {
        avctx = avcodec_alloc_context3(nullptr);
        if (!avctx)
        {
            xlog_err("avcodec_alloc_context3 failed");
            berror = true;
            break;
        }

        ic = is_->ic;

        if (avcodec_parameters_to_context(avctx, ic->streams[stream_index]->codecpar) < 0)
        {
            xlog_err("avcodec_parameters_to_context failed");
            berror = true;
            break;
        }

        avctx->pkt_timebase = ic->streams[stream_index]->time_base;

        codec = avcodec_find_decoder(avctx->codec_id);

        avctx->codec_id = codec->id;

        if ((ret = avcodec_open2(avctx, codec, nullptr)) < 0)
        {
            xlog_err("open codec failed");
            break;
        }

        switch(avctx->codec_type)
        {
            case AVMEDIA_TYPE_AUDIO:
            {
                break;
            }
            case AVMEDIA_TYPE_VIDEO:
            {
                is_->video_stream = stream_index;
                is_->video_st = ic->streams[stream_index];
                if (is_->viddec.init(avctx, is_->videoq.get(), is_->cond_continue_read_thread) < 0)
                {
                    berror = true;
                    xlog_err("decoderInit failed");
                    break;
                }

                xlog_trc("decoder init suc");

                auto lambda_video_trd = [&]()->int
                {
                    return this->videoThread();
                };
                if (is_->viddec.start(lambda_video_trd, "video_deocde") < 0)
                {
                    berror = true;
                    xlog_err("decoderStart failed");
                    break;
                }
                break;
            }
        }
    }
    while (0);

    return ret;
}

int XPlay::streamClose(std::shared_ptr<VideoState> is)
{
    if (is)
    {
        if (is->trd_read)
        {
            is->trd_read->join();
        }
    }

    return 0;
}

int XPlay::getVideoFrame(AVFrame* frame)
{
    int got_picture = 0;

    if ((got_picture = is_->viddec.decodeFrame(frame, nullptr)) < 0)
    {
        return -1;
    }

    if (got_picture)
    {
        // pts
    }

    return got_picture;
}


int Decoder::init(AVCodecContext *avctx_tmp, PacketQueue *queue_tmp, std::shared_ptr<std::condition_variable> empty_queue_cond_tmp)
{
    pkt = av_packet_alloc();
    if (!pkt)
    {
        xlog_err("av_packet_alloc failed");
        return -1;
    }
    avctx = avctx_tmp;
    queue = queue_tmp;
    empty_queue_cond = empty_queue_cond_tmp;
    
    return 0;
}

int Decoder::start(std::function<int()> func, const char *thread_name)
{
    queue->start();
    trd_decoder = std::make_shared<MyThread>(func);
    trd_decoder->start();
    return 0;
}

int Decoder::decodeFrame(AVFrame* frame, AVSubtitle* sub)
{
    xlog_trc("decode frame");

    int ret = 0;

    for (;;)
    {
        // receive frame
        do
        {
            switch (avctx->codec_type)
            {
            case AVMEDIA_TYPE_VIDEO:
            {
                ret = avcodec_receive_frame(avctx, frame);
                break;
            }
            }

            if (ret == AVERROR_EOF)
            {
                avcodec_flush_buffers(avctx);
                return 0;
            }
            if (ret >= 0)
            {
                return 1;
            }

            if (ret == AVERROR(EAGAIN))
            {
                continue;
            }
            else
            {
                break;
            }
        } while (0);

        // get packet

        do
        {
            if (queue->nb_packets == 0)
            {
                empty_queue_cond->notify_one();
            }
            if (packet_pending)
            {
                packet_pending = 0;
            }
            int old_serial = pkt_serial;
            xlog_trc("getting packet");
            if (queue->get(pkt, 1, &pkt_serial) < 0)
            {
                return -1;
            }
            if (old_serial != pkt_serial)
            {
                avcodec_flush_buffers(avctx);
                finished = 0;
                next_pts = start_pts;
                next_pts_tb = start_pts_tb;
            }
            if (queue->serial == pkt_serial)
            {
                break;
            }
            av_packet_unref(pkt);
        } 
        while (1);


        // send packet
        if (avctx->codec_type == AVMEDIA_TYPE_SUBTITLE)
        {
            av_packet_unref(pkt);
        }
        else
        {
            xlog_trc("sending packet");
            if (avcodec_send_packet(avctx, pkt) == AVERROR(EAGAIN))
            {
                xlog_err("error");
            }
            else
            {
                av_packet_unref(pkt);
            }
        }
    }

    return ret;
}

int XPlay::readThread()
{
    xlog_dbg("in");

    int berror = false;

    AVFormatContext *ic = nullptr;
    AVPacket *pkt = nullptr;
    int st_index[AVMEDIA_TYPE_NB]{};
    unsigned int i = 0;
    int ret = 0;
    std::mutex wait_mutex;

    do
    {
        pkt = av_packet_alloc();
        if (!pkt)
        {
            xlog_err("av_packet_alloc failed");
            berror = true;
            break;
        }

        ic = avformat_alloc_context();
        if (!ic)
        {
            xlog_err("avformat_alloc_context failed");
            berror = true;
            break;
        }

        if (avformat_open_input(&ic, is_->filename.c_str(), nullptr, nullptr) < 0)
        {
            xlog_err("avformat_open_input failed");
            berror = true;
            break;
        }

        if (is_->ic != nullptr)
        {
            xlog_err("inner error");
            berror = true;
            break;
        }
        is_->ic = ic;

        for (i = 0; i < array_sizeof(st_index); ++i)
        {
            st_index[i] = -1;
        }

        for (i = 0; i < ic->nb_streams; ++i)
        {
            AVStream *st = ic->streams[i];
            AVMediaType type = st->codecpar->codec_type;
            if (type >= 0 && st_index[type] < 0)
            {
                st_index[type] = i;
            }
        }

        st_index[AVMEDIA_TYPE_VIDEO] = 
            av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, 
                                st_index[AVMEDIA_TYPE_VIDEO], -1, nullptr, 0);

        xlog_trc("video.index=%d", st_index[AVMEDIA_TYPE_VIDEO]);

#if 0
        st_index[AVMEDIA_TYPE_AUDIO] = 
            av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, 
                                st_index[AVMEDIA_TYPE_AUDIO],
                                st_index[AVMEDIA_TYPE_VIDEO], nullptr, 0);

        if (st_index[AVMEDIA_TYPE_AUDIO] >= 0)
        {
            streamComponentOpen(st_index[AVMEDIA_TYPE_AUDIO]);
        }
#endif 

        if (st_index[AVMEDIA_TYPE_VIDEO] >= 0)
        {
            streamComponentOpen(st_index[AVMEDIA_TYPE_VIDEO]);
        }
    
    }
    while (0);

    if (!berror)
    {
        for(;;)
        {
            // break

            // pause change

            // seek

            if (is_->videoq->size > MAX_QUEUE_SIZE ||
                streamHasEnoughPackets(is_->video_st, is_->video_stream, is_->videoq.get()))
            {
                /* wait some time */
                std::unique_lock<std::mutex> lock(wait_mutex);
                xlog_trc("queue full, wait");
                is_->cond_continue_read_thread->wait_for(lock, std::chrono::seconds(10));
                continue;
            }

            ret = av_read_frame(ic, pkt);

            if (ret < 0)
            {
                if ((ret == AVERROR_EOF || avio_feof(ic->pb)) && !is_->eof)
                {
                    if (is_->video_stream >= 0)
                    {
                        is_->videoq->putNullPacket(pkt, is_->video_stream);
                    }
                    is_->eof = 1;
                    xlog_trc("read eof");
                }
                if (ic->pb && ic->pb->error)
                {
                    xlog_err("read error");
                    break;
                }

                std::unique_lock<std::mutex> lock(wait_mutex);
                xlog_trc("read thread wait");
                is_->cond_continue_read_thread->wait_for(lock, std::chrono::seconds(10));
                continue;
            }
            else
            {
                is_->eof = 0;
            }

            if (pkt->stream_index == is_->video_stream
                && !(is_->video_st->disposition & AV_DISPOSITION_ATTACHED_PIC))
            {
                xlog_trc("put packet");
                is_->videoq->put(pkt);
            }
            else
            {
                xlog_trc("discard packet");
                av_packet_unref(pkt);
            }
        }
    }
    
    if (ic && !is_->ic)
    {
        avformat_close_input(&ic);
    }

    av_packet_free(&pkt);

    return 0;
}

int XPlay::videoThread()
{
    xlog_trc("video thread in");

    int ret = 0;
    int berror = false;
    AVFrame* frame = nullptr;
    double duration{};
    double pts{};
    AVRational tb{};
    AVRational frame_rate{};

    frame = av_frame_alloc();
    tb = is_->video_st->time_base;
    frame_rate = av_guess_frame_rate(is_->ic, is_->video_st, nullptr);

    if (!frame)
    {
        xlog_err("frame alloc failed");
        return -1;
    }

    for (;;)
    {
        xlog_trc("getting frame");
        ret = getVideoFrame(frame);
        if (ret < 0)
        {
            xlog_err("get video frame failed");
            berror = true;
            break;
        }
        if (!ret)
        {
            xlog_trc("no frame");
            continue;
        }

        xlog_trc("got frame[%d,%d]", frame->width, frame->height);

        if (frame_rate.num && frame_rate.den)
        {
            duration = av_q2d(frame_rate);
        }
        pts = (frame->pts == AV_NOPTS_VALUE) ? (NAN) : (frame->pts * av_q2d(tb));
        ret = queuePicture(frame, pts, duration, frame->pkt_pos, 0);
        av_frame_unref(frame);
    }

    av_frame_free(&frame);

    return 0;
}

static void SaveYUVBlock(unsigned char *buf, int wrap, int xsize, int ysize, FILE *fp)
{
    for (int i = 0; i < ysize; ++i)
    {
        fwrite (buf + i * wrap, 1, xsize, fp);
    }
}

static void SaveYUV420Frame(const AVFrame *pVFrame, FILE *fp)
{
    SaveYUVBlock((unsigned char*)pVFrame->data[0], pVFrame->linesize[0], pVFrame->width, pVFrame->height, fp);
    SaveYUVBlock((unsigned char*)pVFrame->data[1], pVFrame->linesize[1], pVFrame->width / 2, pVFrame->height / 2, fp);
    SaveYUVBlock((unsigned char*)pVFrame->data[2], pVFrame->linesize[2], pVFrame->width / 2, pVFrame->height / 2, fp);
}

static void SaveYUVNV12Frame (const AVFrame *pVFrame, FILE *fp)
{
    SaveYUVBlock((unsigned char*)pVFrame->data[0], pVFrame->linesize[0], pVFrame->width, pVFrame->height, fp);
    SaveYUVBlock((unsigned char*)pVFrame->data[1], pVFrame->linesize[1], pVFrame->width, pVFrame->height / 2, fp);
}

static void saveFrame(AVFrame *frame)
{
    static FILE *fp = nullptr;
    char filename[] = "output.yuv";

    do
    {
        if (!fp)
        {
            fp = fopen(filename, "wb");
        }
        
        if (!fp)
        {
            break;
        }

        xlog_trc("video.format=%d", frame->format);
        AVPixelFormat format = (AVPixelFormat)frame->format;

        switch (format)
        {
            case AV_PIX_FMT_YUV420P:
            {
                SaveYUV420Frame (frame, fp);
                break;
            }
            case AV_PIX_FMT_NV12:
            {
                SaveYUVNV12Frame(frame, fp);
                break;
            }
            default:
            {
                xlog_err ("unknown pix_fmt=%d\n", (int)format);
                break;
            }
        }

        fflush(fp);
    }
    while (0);
}

int XPlay::queuePicture(AVFrame* src_frame, double pts, double duration, int64_t pos, int serial)
{
    xlog_dbg("push pic");

    saveFrame(src_frame);

    av_frame_unref(src_frame);

    return 0;
}

int XPlay::streamHasEnoughPackets(AVStream* st, int stream_id, PacketQueue* queue)
{
    return stream_id < 0 ||
        queue->abort_request ||
        (st->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
        ((queue->nb_packets > MIN_FRAMES) && (!queue->duration || av_q2d(st->time_base) * queue->duration > 1.0));
}
