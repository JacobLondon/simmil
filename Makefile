TARGET=pse
DISTDIR=../simmil-dist
CXX=g++
CXXFLAGS=-std=c++17 -march=native -O2 -pipe -lSDL2 -lSDL2_image -Wall -Wextra
OBJS=\
	src/pse/ctx_draw.o \
	src/pse/ctx.o \
	src/pse/util.o \
	src/mil.o \
	src/demo.o \
	src/main.o \

.PHONY: clean windows

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS)

windows:
	cp -r assets/ $(DISTDIR)/assets/
	cp Release/pse.exe $(DISTDIR)/pse.exe
	cp libjpeg-9.dll $(DISTDIR)/libjpeg-9.dll
	cp libpng16-16.dll $(DISTDIR)/libpng16-16.dll
	cp libtiff-5.dll $(DISTDIR)/libtiff-5.dll
	cp libwebp-7.dll $(DISTDIR)/libwebp-7.dll
	cp SDL2.dll $(DISTDIR)/SDL2.dll
	cp SDL2_image.dll $(DISTDIR)/SDL2_image.dll
	cp zlib1.dll $(DISTDIR)/zlib1.dll

clean:
	rm -rf $(TARGET) $(OBJS)
