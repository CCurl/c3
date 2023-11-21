app64 := c3
app32 := c3-32

CXX := clang
CXXFLAGS32 := -m32 -O2 
CXXFLAGS64 := -m64 -O3 

srcfiles := $(shell find . -name "*.c")
incfiles := $(shell find . -name "*.h")
LDLIBS   := -lm

all: $(app64) $(app32)

$(app64): $(srcfiles) $(incfiles)
	$(CXX) $(CXXFLAGS64) $(LDFLAGS) -o $(app64) $(srcfiles) $(LDLIBS)
	ls -l $(app64)

$(app32): $(srcfiles) $(incfiles)
	$(CXX) $(CXXFLAGS32) $(LDFLAGS) -o $(app32) $(srcfiles) $(LDLIBS)
	ls -l $(app32)

clean:
	rm -f $(app64) $(app32)

bin: $(app64)
	cp -u -p $(app64) ~/.local/bin/
