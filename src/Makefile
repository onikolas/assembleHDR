DEBUG    = -O3
CXXFLAGS = $(DEBUG) $(shell pkg-config --cflags libraw OpenEXR)
LIBS     = $(shell pkg-config --libs libraw OpenEXR) -lboost_program_options
CXX      =  g++

SOURCES =  assembleHDR.cpp exr.cpp loadraw.cpp

OBJS = $(SOURCES:.cpp=.o)
DEPS = $(SOURCES:.cpp=.d)

TARGET = assembleHDR

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
all:	$(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(DEPS)

-include $(DEPS)
