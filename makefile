ARCH ?= 64
CFLAGS = -O3 -m$(ARCH)
srcfiles := $(shell find . -name "*.cpp")
incfiles := $(shell find . -name "*.h")

c3: $(srcfiles) $(incfiles)
	$(CC) $(CFLAGS) $(srcfiles) -o $@ -lm

run: c3
	./c3

clean:
	rm -f c3

test: c3
	./c3 block-200.c3

bin: c3
	cp -u -p c3 ~/bin/
