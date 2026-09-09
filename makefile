



#CXXFLAGS = -std=c++20 -O0
#SOURCE = a.cpp lexer.cpp
#TARGET = a
#
#all: ${TARGET}
# 
#${TARGET} : ${SOURCE}
#	clang++ ${CXXFLAGS} ${SOURCE} -o ${TARGET}

CXXFLAGS = -std=c++20 -o0 -g -MMD -MP 
SOURCE = main.cpp \
		lexer.cpp \
		parser.cpp \
		semantic_analysis.cpp \
		gen_ir.cpp \
		dump.cpp
		
TARGET = a

OBJECTS = $(SOURCE:.cpp=.o)

all: ${TARGET}

${TARGET}: ${OBJECTS}
	g++ ${OBJECTS} -o ${TARGET}

%.o: %.cpp
	clang++ ${CXXFLAGS} -c $< -o $@

-include $(OBJECTS:.o=.d)

clean:
	rm *.o *.d a