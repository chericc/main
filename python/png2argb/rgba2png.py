from PIL import Image
import sys

def rgba_binary_to_png(input_bin, output_png, width=None, height=None):
    """
    将纯二进制RGBA数据还原为PNG图片
    
    参数:
        input_bin (str): 输入的二进制文件路径
        output_png (str): 输出的PNG图片路径
        width (int): 手动指定图片宽度（可选）
        height (int): 手动指定图片高度（可选）
    """
    try:
        # 读取二进制数据
        with open(input_bin, 'rb') as f:
            rgba_data = f.read()

        # 计算像素总数
        pixel_count = len(rgba_data) // 4  # 每个像素4字节(RGBA)
        
        # 自动推断尺寸（如果未指定width/height）
        if width is None or height is None:
            # 假设为正方形（仅作演示，实际应根据需求修改）
            inferred_size = int(pixel_count ** 0.5)
            width = width or inferred_size
            height = height or inferred_size
            print(f"自动推断尺寸: {width}x{height}")

        # 检查数据是否匹配
        if width * height * 4 != len(rgba_data):
            raise ValueError("数据长度与指定的宽度/高度不匹配")

        # 转换为Pillow图像
        img = Image.frombytes('RGBA', (width, height), rgba_data)
        img.save(output_png)
        
        print(f"还原成功！保存为 {output_png}")

    except Exception as e:
        print(f"错误: {e}", file=sys.stderr)

if __name__ == "__main__":
    if len(sys.argv) != 5:
        print("使用方法: python rgba_to_png.py 输入.bin 输出.png 宽度 高度")
        print("示例: python rgba_to_png.py pixels.bin output.png 800 600")
    else:
        try:
            width = int(sys.argv[3])
            height = int(sys.argv[4])
            rgba_binary_to_png(sys.argv[1], sys.argv[2], width, height)
        except ValueError:
            print("错误: 宽度和高度必须是整数")