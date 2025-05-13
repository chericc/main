from PIL import Image
import sys

def png_to_rgba_binary(input_path, output_path):
    """
    将PNG图片转换为纯二进制RGBA数据
    
    参数:
        input_path (str): 输入的PNG图片路径
        output_path (str): 输出的二进制文件路径
    """
    try:
        # 打开图片并确保是RGBA模式
        img = Image.open(input_path).convert('RGBA')
        
        # 获取图片的原始RGBA数据 (字节串)
        rgba_data = img.tobytes()
        
        # 写入二进制文件
        with open(output_path, 'wb') as f:
            f.write(rgba_data)
        
        print(f"转换成功！输出 {len(rgba_data)} 字节到 {output_path}")
        print(f"图片尺寸: {img.width}x{img.height} (共 {img.width*img.height} 像素)")
        
    except Exception as e:
        print(f"发生错误: {e}", file=sys.stderr)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("使用方法: python png_to_rgba.py 输入.png 输出.bin")
    else:
        png_to_rgba_binary(sys.argv[1], sys.argv[2])
