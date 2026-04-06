CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = katago_game.exe
SOURCES = main.cpp

ifeq ($(OS),Windows_NT)
    TARGET = katago_game.exe
    LDFLAGS = -lws2_32
else
    LDFLAGS = -pthread
endif

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: clean
