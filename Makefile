CC=g++
CFLAGS=-I src/SDL/include 
LDFLAGS=-L src/SDL/lib 
LIBS=-lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer

main: main.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)


