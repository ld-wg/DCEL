CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# default target: build both executables
all: malha draw

malha: main.cpp DCEL.cpp geometry.cpp
	$(CXX) $(CXXFLAGS) -o malha main.cpp DCEL.cpp geometry.cpp

draw: draw.cpp DCEL.cpp geometry.cpp
	$(CXX) $(CXXFLAGS) -o draw draw.cpp DCEL.cpp geometry.cpp

# cleanup
clean:
	rm -f malha draw

.PHONY: all clean 