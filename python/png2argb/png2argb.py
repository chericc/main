import sys
from PIL import Image

def png_to_argb(input_path, output_path):
    try:
        # 打开PNG图片
        img = Image.open(input_path).convert("RGBA")
        width, height = img.size
        
        # 获取像素数据
        pixels = img.tobytes()
        
        # 转换为ARGB格式（每个像素4字节）
        argb_data = bytearray()
        for i in range(0, len(pixels), 4):
            # PNG是RGBA格式，转换为ARGB
            r, g, b, a = pixels[i], pixels[i+1], pixels[i+2], pixels[i+3]
            argb_data.extend([a, r, g, b])
        
        # 写入二进制文件
        with open(output_path, 'wb') as f:
            f.write(argb_data)
            
        print(f"转换成功！输出文件: {output_path}")
        print(f"图片尺寸: {width}x{height}, 总像素: {width*height}")
        print(f"输出数据大小: {len(argb_data)} 字节 (ARGB格式)")
        
    except Exception as e:
        print(f"错误: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("使用方法: python png_to_argb.py 输入.png 输出.bin")
    else:
        png_to_argb(sys.argv[1], sys.argv[2])
