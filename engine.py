import numpy as np
from PIL import Image
import os
import subprocess
import re
import argparse
import sys

parser = argparse.ArgumentParser(description='image_shallow_coding')
parser.add_argument('--mode', type=str, default='Encode', help='Encode, Decode')
parser.add_argument('--lossy', type=int, default=1)
parser.add_argument('--rearrange', type=int, default=0)
parser.add_argument('--reshape', type=int, default=0)
parser.add_argument('--isHaar', type=int, default=0)
parser.add_argument('--image_type', type=str, default='YUV420_UV', help="support YUV420_Y, YU420_UV, YUV420, raw, gray")
parser.add_argument('--dataset_dir', type=str, default='/data/kangbw/test_raw_data/yuv_8bit/yuv')
parser.add_argument('--bin_dir', type=str, default='../bin/')
parser.add_argument('--rec_dir', type=str, default='../rec/')
parser.add_argument('--transLevel', type=int, default=2)
parser.add_argument('--height', type=int, default=2168)
parser.add_argument('--width', type=int, default=3848)
parser.add_argument('--tile_height', type=int, default=8)
parser.add_argument('--tile_width', type=int, default=32)

"""
This Python script is designed to invoke precompiled Encoder and Decoder to test various data. The following are some important parameter descriptions in the script.
lossy means the lossy mode;

reshap means reshape tile into [16, 16] to improve the transLevel;
rearrange means rearrange the raw image in a grid manner;
If the image_type is raw, it is recommanded to set rearrange to 1;
At the same time only one of the rearrange and reshape may be set to 1, if both of them are set to 1, it is equivalent to setting both of them to 0 .

isHaar controls whether to use Haar wavelet instead of the cdf53 wavelet;

transLevel means the level of wavelet, it is recommanded to set the transLevel smaller than round(log2(tile_height)).
"""
def Encode(exe_path, file_path, bin_path, transLevel, width, heigth, tile_width, tile_height, image_type, lossy, rearrange, reshape, isHaar, used_ratio=0.625):
    command = [exe_path, '-i', file_path, '-o', bin_path, '-l', str(transLevel), '-w', str(width), '-h', str(height), '-x', str(tile_width), '-y', str(tile_height), '-t', image_type]
    if lossy:
        command += ['-s']
    if rearrange:
        command += ['-g']
    if reshape:
        command += ['-r']
    if isHaar:
        command += ['-a']
        
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode == 0:
        print('Succeed')
        print(result.stdout)
        # get the compression ratio
        pattern = r"total bits:         (\d+)"
        match = re.search(pattern, result.stdout)
        if match:
            cp_ratio = int(match.group(1))/(os.path.getsize(file_path)*8*used_ratio)
            print(f'Compression ratio: {cp_ratio}')
            return cp_ratio

    else:
        print('Error')
        print(result.stderr)
        
def Decode(exe_path, bin_path, rec_path, transLevel, image_type, lossy, rearrange, reshape, isHaar):
    if not os.path.exists(rec_path):
        with open(rec_path, 'w') as f:
            pass
    command = [exe_path, '-i', bin_path, '-o', rec_path, '-l', str(transLevel), '-t', image_type]
    if lossy:
        command += ['-s']
    if rearrange:
        command += ['-g']
    if reshape:
        command += ['-r']
    if isHaar:
        command += ['-a']
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode == 0:
        print('Succeed')
        print(result.stdout)
    else:
        print('Error')
        print(result.stderr)
    
if __name__ == '__main__': 
    cur_dir = os.getcwd()
    print(f'The current working dir is {cur_dir}')
    args = parser.parse_args()
    dataset_dir = args.dataset_dir
    bin_dir = args.bin_dir
    rec_dir = args.rec_dir
    lossy = args.lossy
    rearrange = args.rearrange
    reshape = args.reshape
    isHaar = args.isHaar
    if not os.path.exists(bin_dir):
        os.makedirs(bin_dir)
    if not os.path.exists(rec_dir):
        os.makedirs(rec_dir)
    transLevel = args.transLevel
    height = args.height
    width = args.width
    if args.tile_height is not None and args.tile_width is not None:
        tile_height = args.tile_height
        tile_width = args.tile_width
    else:
        # the default tile size is the image size
        tile_height = height
        tile_width = width
    if args.mode == 'Encode':
        cp_ratio = 0
        num = 0
        for item in os.listdir(dataset_dir):
            # print(item)
            if args.image_type == 'raw':
                num += 1
                raw_data = np.fromfile(os.path.join(dataset_dir, item), dtype=np.uint16)
                assert len(raw_data) == height*width
                file_path = str(os.path.join(dataset_dir, item))
                bin_path = os.path.join(bin_dir, (item[:-4] + '.bin'))
                cp_ratio += Encode(os.path.join(cur_dir, 'Encoder'), file_path, bin_path, transLevel, width, height, tile_width, tile_height, args.image_type, lossy, rearrange, reshape, isHaar, used_ratio=0.75)
            
            elif args.image_type == 'YUV420_Y':
                num += 1
                raw_data = np.fromfile(os.path.join(dataset_dir, item), dtype=np.uint8)
                assert len(raw_data) == height*width*1.5
                file_path = str(os.path.join(dataset_dir, item))
                bin_path = os.path.join(bin_dir, (item[:-4] + '.bin'))
                cp_ratio += Encode(os.path.join(cur_dir, 'Encoder'), file_path, bin_path, transLevel, width, height, tile_width, tile_height, args.image_type, lossy, rearrange, reshape, isHaar, used_ratio=2/3)
            
            elif args.image_type == 'YUV420_UV':
                num += 1
                raw_data = np.fromfile(os.path.join(dataset_dir, item), dtype=np.uint8)
                assert len(raw_data) == height*width*1.5
                file_path = str(os.path.join(dataset_dir, item))
                bin_path = os.path.join(bin_dir, (item[:-4] + '.bin'))
                cp_ratio += Encode(os.path.join(cur_dir, 'Encoder'), file_path, bin_path, transLevel, width, height, tile_width, tile_height, args.image_type, lossy, rearrange, reshape, isHaar, used_ratio=1/3)
                
            elif args.image_type == 'YUV420':
                num += 1
                raw_data = np.fromfile(os.path.join(dataset_dir, item), dtype=np.uint8)
                assert len(raw_data) == height*width*1.5
                file_path = str(os.path.join(dataset_dir, item))
                bin_path = os.path.join(bin_dir, (item[:-4] + '.bin'))
                cp_ratio += Encode(os.path.join(cur_dir, 'Encoder'), file_path, bin_path, transLevel, width, height, tile_width, tile_height, args.image_type, lossy, rearrange, reshape, isHaar, used_ratio=1)
                
            else:
                print('invalid image_type!')
        cp_ratio = cp_ratio / num
        print(f"Average_compression_ratio: {cp_ratio}")
                
    elif args.mode == 'Decode':
        for item in os.listdir(bin_dir):
            if item[-4:] == '.bin':
                bin_path = os.path.join(bin_dir, item)
                rec_path = os.path.join(rec_dir, ('rec' + item[:-4] + '.raw'))
                Decode(os.path.join(cur_dir, 'Decoder'), bin_path, rec_path, transLevel, args.image_type, lossy, rearrange, reshape, isHaar)
    else:
        print('Mode only supports Encode and Decode')
        




