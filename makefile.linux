LFLAGS=-Wall -funroll-loops -fomit-frame-pointer -pipe -O9
CFLAGS=-Wall `sdl-config --cflags --libs` -funroll-loops -fomit-frame-pointer -pipe -O9 -lSDL_mixer
CC=gcc

C_FILES=src/dlb.c src/linked.c src/sprite.c src/ag.c
OBJ_FILES=src/dlb.o src/linked.o src/sprite.o src/ag.o
OUT_FILE=ag

all:ag

ag: $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $(OUT_FILE) $(OBJ_FILES)

src/dlb.o: src/dlb.c
	$(CC) $(LFLAGS) -c -o $@ $^

src/linked.o: src/linked.c
	$(CC) $(LFLAGS) -c -o $@ $^
	
src/sprite.o: src/sprite.c
	$(CC) $(LFLAGS) -c -o $@ $^

src/ag.o: src/ag.c
	$(CC) $(LFLAGS) -c -o $@ $^

clean:
	rm -f src/*.o
