


CXXFLAGS = -std=c++20 -O0 -g -MMD -MP -Wno-c99-designator

SOURCE = $(wildcard *.cpp)

BUILD_DIR = build
TARGET = $(BUILD_DIR)/a

OBJECTS = $(SOURCE:%.cpp=$(BUILD_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	clang++ $(OBJECTS) -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	clang++ $(CXXFLAGS) -c $< -o $@

-include $(OBJECTS:.o=.d)

clean:
	rm -rf $(BUILD_DIR)