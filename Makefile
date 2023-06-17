CC=g++
CFLAGS=-I src/include
LDFLAGS=-L src/lib
LIBS=-lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -ltinyfiledialogs -lole32 -lcomdlg32 -lSDL2_ttf

main: main.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
