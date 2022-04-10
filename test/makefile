CFLAGS=-I. -I../

all: /tmp/test_sound /tmp/test_mouse /tmp/test_key /tmp/test_io /tmp/test_threads /tmp/test_vm

/tmp/test_sound: test_sound.c ../sound.c
	gcc $(CFLAGS) -o $@ $^

/tmp/test_mouse: test_mouse.c ../mouse.c
	gcc $^ -o $@ $(CFLAGS) -I/usr/include/SDL2 -lSDL2

/tmp/test_key: test_key.c ../key.c
	gcc $^ -o $@ $(CFLAGS) -I/usr/include/SDL2 -lSDL2

/tmp/test_io: test_io.c ../io.c
	gcc $^ -o $@ $(CFLAGS)

/tmp/test_threads: test_threads.c ../threads.c
	gcc $^ -o $@ $(CFLAGS)

/tmp/test_vm: test_vm.c ../threads.c ../vm.c
	gcc $^ -o $@ $(CFLAGS)
