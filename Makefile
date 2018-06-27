


all: capture example

capture: capture.cpp
	g++ -g -o $@ $< -pthread
example: example.cpp
	clang++ -o $@ $< 

clean:
	rm -rf *.o a.out capture example
