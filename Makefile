# 定义编译器
CC = g++

# 定义编译选项
CFLAGS = -Wall -Werror -w -g
MODE := 0

ifeq ($(MODE), 2)
	TARGET = demo
	SRCS = demo.cpp entropyCoding.cpp util.cpp
else
	ifeq ($(MODE), 0)
		TARGET = Encoder
		SRCS = Encoder.cpp entropyCoding.cpp util.cpp
	else
		TARGET = Decoder
		SRCS = Decoder.cpp entropyCoding.cpp util.cpp
	endif
endif


# 定义生成目标
$(TARGET): $(SRCS)
	$(CXX) $(CFLAGS) -o $(TARGET) $(SRCS)

# 定义伪目标"clean"，用于清除生成的目标文件和可执行文件
clean:
	rm -f $(TARGET)

# 声明"clean"为伪目标，即不生成对应的文件
.PHONY: clean
