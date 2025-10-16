#include "xlog.h"

#include "jpeglib.h"
#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <jerror.h>
#include <vector>

/**
 * 使用 libjpeg-turbo 编码 RGB 图像为 JPEG
 * @param rgb_data RGB 图像数据指针
 * @param width 图像宽度
 * @param height 图像高度
 * @param quality 压缩质量 (1-100)
 * @param out_buffer 输出缓冲区（编码后的 JPEG 数据）
 * @param out_size 输出数据大小
 * @return 成功返回 true，失败返回 false
 */
bool encode_jpeg_turbo(const unsigned char* rgb_data, 
                       int width, int height, 
                       int quality,
                       std::vector<unsigned char>& out_buffer) {
    
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    
    // 初始化 JPEG 压缩对象
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    
    // 设置输出到内存缓冲区
    unsigned char* buffer = nullptr;
    unsigned long buffer_size = 0;
    jpeg_mem_dest(&cinfo, &buffer, &buffer_size);
    
    // 设置图像参数
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;  // RGB 3个分量
    cinfo.in_color_space = JCS_RGB;  // RGB 颜色空间
    
    // 设置默认参数
    jpeg_set_defaults(&cinfo);
    
    // 设置压缩质量
    jpeg_set_quality(&cinfo, quality, TRUE);
    
    // 开始压缩
    jpeg_start_compress(&cinfo, TRUE);
    
    // 逐行写入数据
    JSAMPROW row_pointer[1];
    int row_stride = width * 3;  // 每行字节数
    
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = (JSAMPROW)&rgb_data[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }
    
    // 完成压缩
    jpeg_finish_compress(&cinfo);
    
    // 将数据复制到输出缓冲区
    out_buffer.assign(buffer, buffer + buffer_size);
    
    // 清理内存
    free(buffer);
    jpeg_destroy_compress(&cinfo);
    
    return true;
}

/**
 * 保存缓冲区数据到文件（用于测试）
 */
bool save_to_file(const std::vector<unsigned char>& data, const char* filename) {
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        return false;
    }
    
    fwrite(data.data(), 1, data.size(), fp);
    fclose(fp);
    return true;
}

/**
 * 生成测试用的 RGB 图像数据
 */
std::vector<unsigned char> generate_test_image(int width, int height) {
    std::vector<unsigned char> image(width * height * 3);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 3;
            
            // 生成渐变颜色
            image[index] = (unsigned char)(255 * x / width);      // R
            image[index + 1] = (unsigned char)(255 * y / height); // G
            image[index + 2] = (unsigned char)(128);              // B
        }
    }
    
    return image;
}

int main() {
    // 图像参数
    const int width = 640;
    const int height = 480;
    const int quality = 85;
    
    // 生成测试图像数据
    std::vector<unsigned char> rgb_data = generate_test_image(width, height);
    
    // 编码为 JPEG
    std::vector<unsigned char> jpeg_data;
    
    if (encode_jpeg_turbo(rgb_data.data(), width, height, quality, jpeg_data)) {
        printf("编码成功！输出数据大小: %zu 字节\n", jpeg_data.size());
        
        // 保存到文件
        if (save_to_file(jpeg_data, "output.jpg")) {
            printf("已保存到 output.jpg\n");
        } else {
            printf("保存文件失败\n");
        }
    } else {
        printf("编码失败！\n");
        return -1;
    }
    
    return 0;
}
