
#include "framequeue.hpp"

#include "xlog.hpp"

void FrameQueue::unref_item(Frame* vp)
{
    av_frame_unref(vp->frame);
    // avsubtitle_free(&vp->sub);
}

int FrameQueue::init(std::shared_ptr<PacketQueue> pktq_v, int max_size, int keep_last)
{
    pktq = pktq_v;
    queue.resize(max_size);
    keep_last = !!keep_last;
    for (auto& r : queue)
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
    for (auto& r : queue)
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

Frame* FrameQueue::peek()
{
    return &queue[(rindex + rindex_shown) % queue.size()];
}

Frame* FrameQueue::peek_next()
{
    return &queue[(rindex + rindex_shown + 1) % queue.size()];
}

Frame* FrameQueue::peek_last()
{
    return &queue[rindex];
}

Frame* FrameQueue::peek_writable()
{
    std::unique_lock<std::mutex> lock(mutex);
    while (size >= queue.size()
        && !pktq->abort_request())
    {
        xlog_trc("frame queue full, wait");
        cond.wait(lock);
    }

    if (pktq->abort_request())
    {
        return nullptr;
    }

    return &queue[windex];
}

Frame* FrameQueue::peek_readable()
{
    std::unique_lock<std::mutex> lock(mutex);
    while (size - rindex_shown <= 0
        && !pktq->abort_request())
    {
        xlog_trc("read wait");
        cond.wait(lock);
    }

    if (pktq->abort_request())
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
    xlog_trc("framequeue.push, size=%d", size);
    cond.notify_one();
}

void FrameQueue::next()
{
    if (keep_last && !rindex_shown)
    {
        rindex_shown = 1;
        return;
    }

    unref_item(&queue[rindex]);
    if (++rindex == queue.size())
    {
        rindex = 0;
    }

    std::unique_lock<std::mutex> lock(mutex);
    --size;
    xlog_trc("framequque.next, size=%d", size);
    cond.notify_one();
}

int FrameQueue::nb_remaining()
{
    return size - rindex_shown;
}

int64_t FrameQueue::last_pos()
{
    Frame* fp = &queue[rindex];
    if (rindex_shown && fp->serial == pktq->serial())
    {
        return fp->pos;
    }
    else
    {
        return -1;
    }
}