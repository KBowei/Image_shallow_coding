from PIL import Image
import argparse
import numpy as np

parser = argparse.ArgumentParser(description='image_shallow_coding')
parser.add_argument('--mode', type=str, default='Decode')
parser.add_argument('--img_path', type=str, default='/code/rec/recAR0820_20220921191030_FW_T_15029_912_115_AG_5.8_5.8_5.8_DG_1.0_1.0_1.0_F000000F_H_B12_GRBG_3848_2174_H6T0_WF_f83_le.raw')
# parser.add_argument('--img_path', type=str, default='/data/kangbw/test_raw_data/AR0820_20220624153647_10he_GRBG_1924_1084_f1_le_B.raw')
parser.add_argument('--height', type=int, default=2168)
parser.add_argument('--width', type=int, default=3848)

args = parser.parse_args()
# 读取.raw文件的数据
raw_data = np.fromfile(args.img_path, dtype=np.uint8)
print(f'len: {len(raw_data)}')
with open(args.img_path, 'rb') as f:
    data = f.read()
    
# width = int(args.width/2)
# height = int(args.height/2)
width = int(args.width)
height = int(args.height)

# u_data = data[:width*height]
# u_array = np.frombuffer(u_data, dtype=np.uint8).reshape(height, width)
# image = Image.fromarray(u_array).convert('L')
image = Image.frombytes('L', (width, height), data)
# 保存图像
image.save('../rec/lossless.png')