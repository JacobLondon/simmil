TARGET=pse
CXX=g++
CXXFLAGS=-std=c++17 -march=native -O2 -pipe -lSDL2 -lSDL2_image -Wall -Wextra
OBJS=\
	src/pse/ctx_draw.o \
	src/pse/ctx.o \
	src/pse/util.o \
	src/mil.o \
	src/demo.o \
	src/main.o \

.PHONY: clean

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

clean:
	rm -rf $(TARGET) $(OBJS)
