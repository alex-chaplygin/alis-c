OBJS=main.o blancpc.o video.o sound.o mouse.o threads.o vm.o file.o timer.o key.o palette.o io.o sprites.o view.o
CFLAGS = -I/usr/include/SDL2 -DDEBUG -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -g

all: $(OBJS)
	gcc $(OBJS) -o /tmp/alis -lSDL2
