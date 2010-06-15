CFLAGS=-Wall `sdl-config --libs --cflags` -lSDL_mixer
CFLAGS2=-Wall
CC=cc

LIBS=

C_FILES=src/dlb.c src/linked.c src/sprite.c src/ag.c
OBJ_FILES=src/dlb.o src/linked.o src/sprite.o src/ag.o
OUT_FILE=ag

all:ag

ag: $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $(OUT_FILE) $(OBJ_FILES)

src/dlb.o: src/dlb.c
	$(CC) $(CFLAGS2) -c -o $@ $^

src/linked.o: src/linked.c
	$(CC) $(CFLAGS2) -c -o $@ $^
	
src/sprite.o: src/sprite.c
	$(CC) $(CFLAGS2) -c -o $@ $^

src/ag.o: src/ag.c
	$(CC) $(CFLAGS2) -c -o $@ $^
