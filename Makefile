CXX = gcc
CXXFLAGS = -Wall -pthread
TARGET = capture_video
SRC = capture_video.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)