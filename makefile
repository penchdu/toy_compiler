
# 1. 定义编译器，如果系统装了 ccache 就自动用 ccache 包装，没装就退回 clang++
#CCACHE := $(shell command -v ccache 2> /dev/null)
CXX := $(CCACHE) clang++

# 2. 编译与链接参数
# -fsanitize=address 
CXXFLAGS = -std=c++20 -O0 -g -MMD -MP \
	-Wall \
	-Wconstant-logical-operand \
	-Wtautological-compare \
	-Wint-in-bool-context \
	-Wno-c99-designator \
	-Wno-sign-compare \
	-Wno-dangling-else \
	-Wno-unused-parameter \
	-Wno-unused-function \
	-Wno-unused-value \
	-Wno-unused-variable \
	-Wno-writable-strings

# 链接阶段需要加上 ASan 选项
#LDFLAGS += -fsanitize=address

BUILD_DIR = build
TARGET = $(BUILD_DIR)/a

_DUMMY := $(shell chmod +x ./gen_string.sh && ./gen_string.sh)

SOURCE = $(wildcard *.cpp)
OBJECTS = $(SOURCE:%.cpp=$(BUILD_DIR)/%.o)

all: $(TARGET)

# 链接阶段：使用 CXX + OBJECTS + LDFLAGS 生成最终可执行文件
$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJECTS) -o $@

# 编译阶段：仅使用 CXXFLAGS 生成 .o 文件
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(OBJECTS:.o=.d)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean