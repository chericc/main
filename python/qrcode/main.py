# do_dir will traverse all files in a directory and print & write their qrcode info.

from PIL import Image
import os
from pyzbar import pyzbar

def decode_qr_code(code_img_path):
    if not os.path.exists(code_img_path):
        raise FileExistsError(code_img_path)
    return pyzbar.decode(Image.open(code_img_path), symbols=[pyzbar.ZBarSymbol.QRCODE])

def str_qr_code(code_img_path):
    result = decode_qr_code(code_img_path)
    if len(result):
        return result[0].data.decode("utf-8")
    else:
        return str("")

def do_dir(dir_path: str):
    dir_path = os.path.abspath(dir_path)
    filenames = os.listdir(dir_path)
    filenames.sort()
    file_output = open(file=dir_path + "_result.txt", mode="w")

    count: int = 0
    for file in filenames:
        path = dir_path + "/" + file
        qrcode = str_qr_code(path)
        str = "index={0},filename=[{1}],code=[{2}]".format(count, file, qrcode)
        print(str)
        file_output.write(str + "\n")
        count += 1
    
    file_output.close()

def main():
    do_dir("/home/test/tmp/pic")
    do_dir("/home/test/tmp/pic2")

if __name__ == "__main__":
    main()