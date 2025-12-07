CXX := g++
CXXFLAGS := -Wall -Wextra -pthread -std=c++17
TARGET := mavsdk_test
SRCS := mavsdk_test.cpp
OBJS := $(SRCS:.cpp=.o)

# MAVSDK flags
MAVSDK_CFLAGS := $(shell pkg-config --cflags mavsdk 2>/dev/null)
MAVSDK_LIBS := $(shell pkg-config --libs mavsdk 2>/dev/null)

ifeq ($(strip $(MAVSDK_LIBS)),)
$(warning MAVSDK not found via pkg-config; trying manual flags)
MAVSDK_CFLAGS := -I/usr/local/include -I/usr/include -I/usr/include/mavsdk
MAVSDK_LIBS := -L/usr/lib -lmavsdk
else
# Add mavsdk directory to include path so plugin headers can find plugin_base.h
MAVSDK_CFLAGS += -I/usr/include/mavsdk
endif

CXXFLAGS += $(MAVSDK_CFLAGS)
LDFLAGS += $(MAVSDK_LIBS)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean
