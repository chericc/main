# ffmpeg_ts解析

## reference

libavformat/mpegts.c

mpeg-ts-iso-13818-1

## code

```c
static int mpegts_read_header(AVFormatContext *s)
{
    MpegTSContext *ts = s->priv_data;
    AVIOContext *pb   = s->pb;
    int64_t pos, probesize = s->probesize;
    int64_t seekback = FFMAX(s->probesize, (int64_t)ts->resync_size + PROBE_PACKET_MAX_BUF);

    s->internal->prefer_codec_framerate = 1;

    if (ffio_ensure_seekback(pb, seekback) < 0)
        av_log(s, AV_LOG_WARNING, "Failed to allocate buffers for seekback\n");

    pos = avio_tell(pb);
    ts->raw_packet_size = get_packet_size(s);
    if (ts->raw_packet_size <= 0) {
        av_log(s, AV_LOG_WARNING, "Could not detect TS packet size, defaulting to non-FEC/DVHS\n");
        ts->raw_packet_size = TS_PACKET_SIZE;
    }
    ts->stream     = s;
    ts->auto_guess = 0;

    if (s->iformat == &ff_mpegts_demuxer) {
        /* normal demux */

        /* first do a scan to get all the services */
        seek_back(s, pb, pos);

        mpegts_open_section_filter(ts, SDT_PID, sdt_cb, ts, 1);
        mpegts_open_section_filter(ts, PAT_PID, pat_cb, ts, 1);
        mpegts_open_section_filter(ts, EIT_PID, eit_cb, ts, 1);

        handle_packets(ts, probesize / ts->raw_packet_size);
        /* if could not find service, enable auto_guess */

        ts->auto_guess = 1;

        av_log(ts->stream, AV_LOG_TRACE, "tuning done\n");

        s->ctx_flags |= AVFMTCTX_NOHEADER;
    } else {
    ...
    }
}
```

```c

/* 探测得到ts包的大小 */
static int get_packet_size(AVFormatContext* s)
{
    int score, fec_score, dvhs_score;
    int margin;
    int ret;

    /*init buffer to store stream for probing */
    uint8_t buf[PROBE_PACKET_MAX_BUF] = {0};
    int buf_size = 0;
    int max_iterations = 16;

    while (buf_size < PROBE_PACKET_MAX_BUF && max_iterations--) {
        ret = avio_read_partial(s->pb, buf + buf_size, PROBE_PACKET_MAX_BUF - buf_size);
        if (ret < 0)
            return AVERROR_INVALIDDATA;
        buf_size += ret;

        score      = analyze(buf, buf_size, TS_PACKET_SIZE,      0);
        dvhs_score = analyze(buf, buf_size, TS_DVHS_PACKET_SIZE, 0);
        fec_score  = analyze(buf, buf_size, TS_FEC_PACKET_SIZE,  0);
        av_log(s, AV_LOG_TRACE, "Probe: %d, score: %d, dvhs_score: %d, fec_score: %d \n",
            buf_size, score, dvhs_score, fec_score);

        margin = mid_pred(score, fec_score, dvhs_score);

        if (buf_size < PROBE_PACKET_MAX_BUF)
            margin += PROBE_PACKET_MARGIN; /*if buffer not filled */

        if (score > margin)
            return TS_PACKET_SIZE;
        else if (dvhs_score > margin)
            return TS_DVHS_PACKET_SIZE;
        else if (fec_score > margin)
            return TS_FEC_PACKET_SIZE;
    }
    return AVERROR_INVALIDDATA;
}

static int analyze(const uint8_t *buf, int size, int packet_size,
                   int probe)
{
    int stat[TS_MAX_PACKET_SIZE];
    int stat_all = 0;
    int i;
    int best_score = 0;

    memset(stat, 0, packet_size * sizeof(*stat));

    for (i = 0; i < size - 3; i++) {
        if (buf[i] == 0x47) {
            int pid = AV_RB16(buf+1) & 0x1FFF;
            int asc = buf[i + 3] & 0x30;
            if (!probe || pid == 0x1FFF || asc) {
                int x = i % packet_size;
                stat[x]++;
                stat_all++;
                if (stat[x] > best_score) {
                    best_score = stat[x];
                }
            }
        }
    }

    return best_score - FFMAX(stat_all - 10*best_score, 0)/10;
}

```

## mpegts_read_header

- 获取包大小（普通应用可以直接以188计）get_packet_size
  - 通过探测的方式猜测包的大小
- 注册section处理函数（sdt、pat、eit）mpegts_open_section_filter
- 根据数据大小计算出总包数，处理包；handle_packets
  - 检测是否有seek（pos是否变化了）
  - 逐个读取包，read_packet
    - 读取数据
    - 检查首字节是否为同步字节，不是的话就需要同步 mpegts_resync
      - 从当前位置往前最多一个包大小的位置开始找（最多比如64KB）同步字节
      - 重新获取包大小
      - 移动至同步字节位置
      - 通知上层需要重试（EAGAIN）
  - 逐个处理包，handle_packet
    - 读取PID、payload_unit_start_indicator、adaptation_field_control
    - 对于有 payload_unit_start_indicator 的，检测这个PID是否被丢弃，若被丢弃，不做后续处理

