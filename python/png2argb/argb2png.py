import sys
from PIL import Image

def argb_to_png(input_path, output_path, width, height):
    try:
        # 读取ARGB二进制数据
        with open(input_path, 'rb') as f:
            argb_data = f.read()
        
        # 检查数据大小是否匹配
        expected_size = width * height * 4
        if len(argb_data) != expected_size:
            raise ValueError(f"数据大小不匹配，预期 {expected_size} 字节，实际 {len(argb_data)} 字节")
        
        # 转换为RGBA格式（PIL需要的格式）
        rgba_data = bytearray()
        for i in range(0, len(argb_data), 4):
            a, r, g, b = argb_data[i], argb_data[i+1], argb_data[i+2], argb_data[i+3]
            rgba_data.extend([r, g, b, a])
        
        # 创建新图像
        img = Image.frombytes('RGBA', (width, height), bytes(rgba_data))
        img.save(output_path, 'PNG')
        
        print(f"转换成功！输出文件: {output_path}")
        print(f"图片尺寸: {width}x{height}, 总像素: {width*height}")
        
    except Exception as e:
        print(f"错误: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 5:
        print("使用方法: python argb_to_png.py 输入.bin 输出.png 宽度 高度")
        print("示例: python argb_to_png.py pixels.bin output.png 800 600")
    else:
        try:
            width = int(sys.argv[3])
            height = int(sys.argv[4])
            argb_to_png(sys.argv[1], sys.argv[2], width, height)
        except ValueError:
            print("错误: 宽度和高度必须是整数")
