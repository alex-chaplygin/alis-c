OBJS=main.o graphics.o key.o threads.o blancpc.o file.o script.o memory.o sprite.o interpret.o call.o array.o get.o store.o math.o var.o render.o palette.o append.o vector.o image.o scene.o misc.o draw.o project.o
CFLAGS = -I/usr/include/SDL2 -DDEBUG -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -g

all: $(OBJS)
	gcc $(OBJS) -o /tmp/alis -lSDL2
