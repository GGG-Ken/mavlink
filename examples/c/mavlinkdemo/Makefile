# 目标名称
TARGET := libmavlinkdemo.so

# 如果要编译动态库，把这个值置为1
MAKE_DYNAMIC := 1

## 如果要交叉编译，可以在环境变量里定义CROSS_COMPILE的路径。
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
STRIP := $(CROSS_COMPILE)strip

# 包含目录
INC_DIR := -I$(CURDIR)/mavlinkdemo
INC_DIR += -I$(CURDIR)/mavlinkdemo/utils

# 链接库
LD_LIBS := -pthread -lrt

# 使用-g增加gdb调试参数
CFLAGS := -g -fPIC

# 如果要编译为动态链接库，需要在LDFLAGS里增加-shared标志。
ifeq ($(MAKE_DYNAMIC), 1)
	LDFLAGS := -shared
else
	LDFLAGS :=
endif

# 查找源文件
SRC_C := $(wildcard *.c)

# 目标文件和依赖文件
OBJS_C := $(subst .c,.o, $(SRC_C))
D_FILE_C := $(subst .c,.d, $(SRC_C))

# 生成动态库
$(TARGET): $(OBJS_C)
ifeq ($(MAKE_DYNAMIC), 1)
	$(CC) $(LDFLAGS) -o $(TARGET) $^ $(LD_LIBS)
#	$(CXX) -o $(TARGET) $(LD_LIBS) $(LOCAL_LDLIBS) $(LIB_EX_DIR) $^
#	$(CXX) -o $(TARGET) $^ `pkg-config opencv4 --cflags --libs`
#	$(CXX) $(LDFLAGS) -o $(TARGET) $^ $(LD_LIBS)
endif
#如果要编译为静态库,不使用CXX,而是使用ar来生成目标文件
#	ar rcs $@ $(OBJS)

-include $(D_FILE_C)

# 生成目标文件
#-MMD选项会生成".d"文件.如果不需要,可以把这个编译选项去掉.
%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $(INC_DIR) $<
# $(CC) $(CFLAGS) -MMD -c -o $@ $(INC_DIR) $<

clean:
	rm -f $(TARGET) $(D_FILE_C) $(OBJS_C)
