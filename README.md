# Image_shallow_coding
image shallow coding supporting raw/yuv420 data

通过Makefile 构建和编译工程Encoder，Decoder

编译Encoder make MODE=0

编译Decoder make MODE=1

编译好后通过调用engine.py测试编解码

engine.py 参数解释：

mode：		Encode/Decode
lossy：		1有损0无损
rearrange：	田字格重排，用于raw格式图像，1打开
reshape：	顺序重排为16*16的块，用于增加小波分辨率，1打开
isHaar：		是否使用Haar小波代替cdf53小波，1使用
image_type：	编码图像格式，支持raw，gray，YUV220，YUV420_Y，YUV420_UV
dataset_dir：	测试数据集路径
bin_dir：		码流存储路径
rec_dir：		解码图像存储路径
height：		图像高
width：		图像宽
transLevel：	小波变换级数
tile_height：	tile的高
tile_width：	tile的宽

也可直接在命令行调用Encoder/Decoder进行测试

Encoder参数：
[-i <imageFile>] [-o <outputFileName>] [-t <image_type>] [-l <transLevel>] [-w <width>] [-h <height>] [-x <tile_width>] [-y <tile_height>] [-s] [-g] [-r] [-a]
-s表示有损 -g表示rearrange为true -r表示reshape为true -a表示isHaar为true

Decoder参数：
[-i <imageFile>] [-o <outputFileName>] [-t <image_type>] [-l <transLevel>] [-s] [-g] [-r] [-a]

例如：
Encoder -i path_of_image -o path_of_bin -t raw -l 3 -w 3848 -h 2168 -x 1 -y 256 -g -a
表示编码一张raw格式图像，使用田字格重排和Haar小波变换

解码器的设置需要和编码器保持一致：
Decoder -i path_of_bin -o path_of_rec -t raw -l 3 -g -a

Q&A

*rearrange和reshape是否可以同时开启？

同时开启不会报错，但是效果等于都不开启

*rearrange在YUV420格式图片是否有效？

在YUV420格式下，默认不开启rearrange，即使开启也不会报错，但是效果和不开启一样

*有损模式如何设置？

目前有损模式基于比特平面编码技术，对于tile_height==1的tile，默认丢弃H0的detail数据中低四位数据；对于tile_height!=1的tile，默认丢弃HH0的detail数据中的低二位数据
可以在代码util.cpp的entropyEncoding和entropyDecoding中修改对应位置的值。

*重复编码同一张图片码流会叠加还是覆盖？

覆盖

*小波变换级数如何设置？

推荐transLevel <= round(log2(min(tile_height, tile_width))
