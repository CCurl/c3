app := c3

CXX := clang++
CXXFLAGS := -m64 -O3 -DIS_LINUX

srcfiles := $(shell find . -name "*.cpp")
incfiles := $(shell find . -name "*.h")
LDLIBS   := -lm

all: $(app)

$(app): $(srcfiles) $(incfiles)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(app) $(srcfiles) $(LDLIBS)
	ls -l $(app)

clean:
	rm -f $(app)

rebuild: clean all

bin: $(app)
	cp -u -p $(app) ~/bin/
