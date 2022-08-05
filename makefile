OBJS=main.o graphics.o key.o objects.o blancpc.o file.o class.o io.o memory.o sprite.o interpret.o call.o array.o get.o store.o math.o var.o render.o palette.o append.o vector.o window.o misc.o draw.o project.o sound.o text.o mouse.o str.o collision.o res.o
CFLAGS = -I/usr/include/SDL2 -DDEBUG -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -g

all: $(OBJS)
	gcc $(OBJS) -o /tmp/alis -lSDL2
