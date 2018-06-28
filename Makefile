


all: capture example replay

capture: capture.cpp
	g++ -g -o $@ $< -pthread
example: example.cpp
	clang++ -o $@ $< 
replay: replay.cpp
	g++ -g -o $@ $<
clean:
	rm -rf *.o a.out capture example replay test_input
