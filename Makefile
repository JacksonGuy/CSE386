cc = g++
CFLAGS = -g
SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, %.o, $(SOURCES))
LINKS = -lglut -lGL
EXE = main

%.o : %.cpp
	$(cc) $(CFLAGS) -c -o $@ $<

all: $(OBJS)
	$(cc) -o $(EXE) $^ $(CFLAGS) $(LINKS)

clean:
	rm -rf ./*.o
	rm main
