#pragma once


/* Common struct for handling all types of decoded data and allocated render buffers. */
class Frame
{
public:
    AVFrame* frame{ nullptr };
    // AVSubtitle sub{};
    int serial{ 0 };
    double pts{ 0.0 };           /* presentation timestamp for the frame */
    double duration{ 0.0 };      /* estimated duration of the frame */
    int64_t pos{ 0 };          /* byte position of the frame in the input file */
    int width{ 0 };
    int height{ 0 };
    int format{ 0 };
    AVRational sar{};
    int uploaded{ 0 };
    int flip_v{ 0 };
};
