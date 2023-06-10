

#include "xplay.hpp"
#include "xlog.hpp"

#define array_sizeof(array) ((sizeof(array))/(sizeof(array[0])))

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
        if (_is)
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

    if (!_is)
    {
        xlog_err("not opened");
    }

    ret = doClose();

    return ret;
}

int XPlay::doOpen(const OptValues &opt)
{
    int ret = 0;

    if (_is)
    {
        doClose();
    }

    if (_is)
    {
        xlog_err("not null");
        return -1;
    }

    _is = streamOpen(opt);

    /* 赋值之后再启动线程 */
    if (_is && _is->trd_read)
    {
        _is->trd_read->start();
    }
    
    return _is ? 0 : -1;
}

int XPlay::doClose()
{
    int ret = 0;

    if (_is)
    {
        if (streamClose(_is) < 0)
        {
            xlog_err("close failed");
            ret = -1;
        }
    }

    _is.reset();

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

        if (is->pictq->init(is->videoq, 16, true) < 0)
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
        is->trd_read = std::make_shared<XThread>(lambda_readtrd);
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

        ic = _is->ic;

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
                _is->video_stream = stream_index;
                _is->video_st = ic->streams[stream_index];
                if (_is->viddec.init(avctx, _is->videoq.get(), _is->cond_continue_read_thread) < 0)
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
                if (_is->viddec.start(lambda_video_trd, "video_deocde") < 0)
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

    if ((got_picture = _is->viddec.decodeFrame(frame, nullptr)) < 0)
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
    trd_decoder = std::make_shared<XThread>(func);
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
            else 
            {
                int old_serial = pkt_serial;
                xlog_trc("getting packet");
                if (queue->get(pkt, 1, &pkt_serial) < 0)
                {
                    return -1;
                }
                xlog_trc("pkt: serial=%d(old=%d)", pkt_serial, old_serial);
                if (old_serial != pkt_serial)
                {
                    avcodec_flush_buffers(avctx);
                    finished = 0;
                    next_pts = start_pts;
                    next_pts_tb = start_pts_tb;
                }
            }
            if (queue->serial == pkt_serial)
            {
                break;
            }
            av_packet_unref(pkt);
        } 
        while (1);


        // send packet
        if (avctx->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            xlog_trc("sending packet(codec_type=%d)", avctx->codec_type);
            if (avcodec_send_packet(avctx, pkt) == AVERROR(EAGAIN))
            {
                xlog_err("error");
            }
            else
            {
                av_packet_unref(pkt);
            }
        }
        else
        {
            xlog_trc("packet ignored(codec_type=%d)", avctx->codec_type);
            av_packet_unref(pkt);
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

        if (avformat_open_input(&ic, _is->filename.c_str(), nullptr, nullptr) < 0)
        {
            xlog_err("avformat_open_input failed");
            berror = true;
            break;
        }

        if (_is->ic != nullptr)
        {
            xlog_err("inner error");
            berror = true;
            break;
        }
        _is->ic = ic;

        _is->max_frame_duration = (ic->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;

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

            if (_is->videoq->size > MAX_QUEUE_SIZE ||
                streamHasEnoughPackets(_is->video_st, _is->video_stream, _is->videoq.get()))
            {
                /* wait some time */
                std::unique_lock<std::mutex> lock(wait_mutex);
                xlog_trc("packet queue full, wait");
                _is->cond_continue_read_thread->wait_for(lock, std::chrono::seconds(10));
                continue;
            }

            ret = av_read_frame(ic, pkt);

            if (ret < 0)
            {
                if ((ret == AVERROR_EOF || avio_feof(ic->pb)) && !_is->eof)
                {
                    if (_is->video_stream >= 0)
                    {
                        _is->videoq->put_null_packet(pkt, _is->video_stream);
                    }
                    _is->eof = 1;
                    xlog_trc("read eof");
                }
                if (ic->pb && ic->pb->error)
                {
                    xlog_err("read error");
                    break;
                }

                std::unique_lock<std::mutex> lock(wait_mutex);
                xlog_trc("read thread wait");
                _is->cond_continue_read_thread->wait_for(lock, std::chrono::seconds(10));
                continue;
            }
            else
            {
                _is->eof = 0;
            }

            if (pkt->stream_index == _is->video_stream
                && !(_is->video_st->disposition & AV_DISPOSITION_ATTACHED_PIC))
            {
                xlog_trc("put packet(si=%d)", pkt->stream_index);
                _is->videoq->put(pkt);
            }
            else
            {
                xlog_trc("discard packet(si=%d)", pkt->stream_index);
                av_packet_unref(pkt);
            }
        }
    }
    
    if (ic && !_is->ic)
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
    tb = _is->video_st->time_base;
    frame_rate = av_guess_frame_rate(_is->ic, _is->video_st, nullptr);

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
            duration = av_q2d(AVRational{frame_rate.den, frame_rate.num});
        }
        else 
        {
            duration = 0;
        }

        pts = (frame->pts == AV_NOPTS_VALUE) ? (NAN) : (frame->pts * av_q2d(tb));
        ret = queuePicture(frame, pts, duration, frame->pkt_pos, _is->videoq->serial);
        av_frame_unref(frame);

        if (ret < 0)
        {
            xlog_trc("video thread break");
            break;
        }
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

    Frame* vp = nullptr;

    vp = _is->pictq->peek_writable();

    if (!vp)
    {
        return -1;
    }

    vp->sar = src_frame->sample_aspect_ratio;
    vp->uploaded = 0;

    vp->width = src_frame->width;
    vp->height = src_frame->height;
    vp->format = src_frame->format;

    vp->pts = pts;
    vp->duration = duration;
    vp->pos = pos;
    vp->serial = serial;

    //saveFrame(src_frame);
    //av_frame_unref(src_frame);
    
    xlog_trc("w%d:h%d:pts%.2f:serial%d", 
        vp->width, vp->height, vp->pts, vp->serial);

    av_frame_move_ref(vp->frame, src_frame);
    _is->pictq->push();

    return 0;
}

int XPlay::streamHasEnoughPackets(AVStream* st, int stream_id, PacketQueue* queue)
{
    return stream_id < 0 ||
        queue->abort_request ||
        (st->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
        ((queue->nb_packets > MIN_FRAMES) && (!queue->duration || av_q2d(st->time_base) * queue->duration > 1.0));
}

int XPlay::refresh(RefreshState *state)
{
    if (!_is)
    {
        xlog_err("null");
        return -1;
    }

    doRefresh(state);

    return 0;
}

int XPlay::doRefresh(RefreshState *state)
{
    if (_is->pictq->nb_remaining() == 0)
    {
        xlog_trc("picq empty");
    }
    else
    {
        Frame* vp = nullptr;
        Frame* last_vp = nullptr;
        double last_duration = 0.0;
        double duration = 0.0;
        double delay = 0.0;
        double time = 0.0;

        for(;;)
        {
            last_vp = _is->pictq->peek_last();
            vp = _is->pictq->peek();

            xlog_trc("serial(vp:%d,last_vp:%d,queue:%d)", 
                vp->serial, last_vp->serial, _is->videoq->serial);
            if (vp->serial != _is->videoq->serial)
            {
                xlog_trc("serial changed, next");
                _is->pictq->next();
                continue;
            }

            if (last_vp->serial != vp->serial)
            {
                _is->refreshctx.frame_timer = av_gettime_relative() / 1000000.0;
            }

            last_duration = vp_duration(last_vp, vp);
            delay = compute_target_delay(last_duration);

            time = av_gettime_relative() / 1000000.0;

            xlog_trc("time=%.2f,frame_timer=%.2f,delay=%.2f",
                time, _is->refreshctx.frame_timer, delay);
                
            if (time < _is->refreshctx.frame_timer + delay)
            {
                state->remaining_time = std::min(_is->refreshctx.frame_timer + delay - time, 
                    state->remaining_time);
                break;
            }

            _is->refreshctx.frame_timer += delay;
            if (delay > 0 && time - _is->refreshctx.frame_timer > AV_SYNC_THRESHOLD_MAX)
            {
                _is->refreshctx.frame_timer = time;
            }

            _is->pictq->next();
            _is->refreshctx.force_refresh = true;

            break;
        }
    }

    _is->refreshctx.force_refresh = false;

    state->frame = _is->pictq->peek_last();

    if (state->frame)
    {
        xlog_trc("state: [pts=%.2f, duration=%.2f, remaining_time=%.2f]", 
            state->frame->pts,
            state->frame->duration,
            state->remaining_time);
    }
    else 
    {
        xlog_trc("state: [null, remaining_time=%.2f]",
            state->remaining_time);
    }

    return 0;
}

double XPlay::vp_duration(Frame *vp, Frame *nextvp)
{
    auto max_frame_duration = _is->max_frame_duration;
    if (vp->serial == nextvp->serial) {
        double duration = nextvp->pts - vp->pts;
        if (isnan(duration) || duration <= 0 || duration > max_frame_duration)
            return vp->duration;
        else
            return duration;
    }
    else {
        return 0.0;
    }
}

double XPlay::compute_target_delay(double delay)
{
    return delay;
}