#include <cmath>
#include <cstdio>

typedef struct {
    int x0; // 0 - 10000
    int y0;
    int x1;
    int y1;
} gsf_codec_scale_rect_t;

typedef struct {
    int num; // 分子（宽）
    int den; // 分母（高）
} ratio;

static void mpp_vdec_adjust_scale_rect(gsf_codec_scale_rect_t *rect, 
    int maxw, int maxh,  // 
    ratio scale, ratio ra)
{
    const int original_w = rect->x1 - rect->x0;
    const int original_h = rect->y1 - rect->y0;
    const int midx = rect->x0 + original_w / 2;
    const int midy = rect->y0 + original_h / 2;

    // 直接放大
    int scaled_w = original_w * scale.num / scale.den;
    int scaled_h = original_h * scale.num / scale.den;

    // 将放大后的画面大小限制到最大画面
    if (scaled_w > maxw) {
        scaled_w = maxw;
    }
    if (scaled_h > maxh) {
        scaled_h = maxh;
    }

    // 将放大后的画面挪动到最大画面中（如果超出了的话）
    
    int x0 = midx - scaled_w / 2;
    int x1 = midx + scaled_w / 2;
    int y0 = midy - scaled_h / 2;
    int y1 = midy + scaled_h / 2;
    
    // 挪动中心点
    int new_midx = midx;
    int new_midy = midy;
    if (x0 < 0) {
        new_midx += abs(x0);
    }
    if (x1 > maxw) {
        new_midx -= abs(x1 - maxw);
    }
    if (y0 < 0) {
        new_midy += abs(y0);
    }
    if (y1 > maxh) {
        new_midy -= abs(y1 - maxh);
    }

    // 此时放大后的画面一定在画面中
    // 将画面裁剪到指定比例（缩小）

    // 如果比例合适，则两个值应相等
    int w_factor = scaled_w * ra.den;
    int h_factor = scaled_h * ra.num;
    
    if (w_factor > h_factor) {
        scaled_w = scaled_h * ra.num / ra.den;
    } else {
        scaled_h = scaled_w * ra.den / ra.num;
    }

    x0 = new_midx - scaled_w / 2;
    x1 = new_midx + scaled_w / 2;
    y0 = new_midy - scaled_h / 2;
    y1 = new_midy + scaled_h / 2;

    rect->x0 = x0;
    rect->y0 = y0;
    rect->x1 = x1;
    rect->y1 = y1;

    return ;
}

int main()
{
    gsf_codec_scale_rect_t rect = {
        (int)(0.38329365 * 10000),
        (int)(0.334080874 * 10000),
        (int)(0.6191534 * 10000),
        (int)(0.70118010 * 10000)
    };

    ratio scale = {};
    ratio graph = {};
    scale.num = 4;
    scale.den = 3;
    graph.num = 1;
    graph.den = 1;

    mpp_vdec_adjust_scale_rect(&rect, 10000, 10000, scale, graph);

    printf("[%d,%d,%d,%d]\n", rect.x0, rect.y0, rect.x1, rect.y1);
    return 0;
}