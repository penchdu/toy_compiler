

CXXFLAGS = -std=c++20 -O0 -g -MMD -MP

SOURCE = main.cpp \
         lexer.cpp \
         parser.cpp \
         semantic_analysis.cpp \
         gen_ir.cpp \
         dump.cpp

BUILD_DIR = build

TARGET = $(BUILD_DIR)/a

OBJECTS = $(SOURCE:%.cpp=$(BUILD_DIR)/%.o)

all: $(TARGET)


$(TARGET): $(OBJECTS)
	clang++ $(OBJECTS) -o $(TARGET)


$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	clang++ $(CXXFLAGS) -c $< -o $@


-include $(OBJECTS:.o=.d)


clean:
	rm -rf $(BUILD_DIR)
	