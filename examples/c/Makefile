# 目标名称
TARGET := main

# 如果要编译动态库，把这个值置为1
MAKE_DYNAMIC := 1

## 如果要交叉编译，可以在环境变量里定义CROSS_COMPILE的路径。
CXX := $(CROSS_COMPILE)g++
CC := $(CROSS_COMPILE)gcc
STRIP := $(CROSS_COMPILE)strip

# 包含目录
INC_DIR := -I$(CURDIR)/mavlinkdemo

# 链接库
LD_LIBS := -pthread -lrt

# 使用-g增加gdb调试参数
CFLAGS := -g

# 如果要编译为动态链接库，需要在LDFLAGS里增加-shared标志。
# 同时，把生成的库名字修改，增加lib前缀
ifeq ($(MAKE_DYNAMIC), 1)
	LDFLAGS := -shared
	TARGET := lib$(TARGET).so
else
	LDFLAGS :=
endif

# 查找源文件
SRC := $(wildcard examples/c/*.cpp)

SRC_C := $(wildcard examples/c/mavlinkdemo/*.c)
OBJS := $(subst .cpp,.o, $(SRC))
OBJS_C := $(subst .c,.o, $(SRC_C))
D_FILE := $(subst .cpp,.d, $(SRC))
D_FILE_C := $(subst .c,.d, $(SRC_C))

$(TARGET): $(OBJS) $(OBJS_C)
ifeq ($(MAKE_DYNAMIC), 1)
	$(CXX) $(LDFLAGS) -o $(TARGET) $^ $(LD_LIBS)
else
	$(CXX) -o $(TARGET) $^ $(LD_LIBS)
endif

-include $(D_FILE) $(D_FILE_C)

%.o : %.cpp
	$(CXX) $(CFLAGS) -MMD -c -o $@ $(INC_DIR) $<

%.o : %.c
	$(CC) $(CFLAGS) -MMD -c -o $@ $(INC_DIR) $<

clean:
	rm -f $(TARGET) $(D_FILE) $(D_FILE_C) $(OBJS) $(OBJS_C)
