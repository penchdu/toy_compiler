

# 1. 定义编译器，如果系统装了 ccache 就自动用 ccache 包装，没装就退回 clang++
CCACHE := $(shell command -v ccache 2> /dev/null)
CXX := $(CCACHE) clang++

CXXFLAGS = -std=c++20 -O0 -g -MMD -MP -Wno-c99-designator
BUILD_DIR = build
TARGET = $(BUILD_DIR)/a

_DUMMY := $(shell chmod +x ./gen_string.sh && ./gen_string.sh)

SOURCE = $(wildcard *.cpp)
OBJECTS = $(SOURCE:%.cpp=$(BUILD_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJECTS) -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(OBJECTS:.o=.d)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean



