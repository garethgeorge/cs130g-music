CXX=g++
CFLAGS=-std=c++14
OBJECTS=main.o fileformats.o

program: $(OBJECTS)
	$(CXX) $(CFLAGS) -o program $(OBJECTS)

fileformats.o: fileformats.h fileformats.cpp synth2.h
	$(CXX) $(CFLAGS) -c fileformats.cpp	-o fileformats.o

main.o: main.cpp synth2.h
	$(CXX) $(CFLAGS) -c main.cpp -o main.o

clean:
	rm *.o program