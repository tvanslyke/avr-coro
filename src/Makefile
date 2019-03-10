CC=avr-gcc-8
CXX=avr-g++-8
AR=avr-ar


CXXFLAGS=-std=c++17 -O2 -fmax-errors=5 -Wall -Wextra -ffunction-sections -fdata-sections -w -I./


libtimcoro.a: Coroutine.o
	$(AR) rcs libtimcoro.a Coroutine.o

Coroutine.o: Coroutine.cpp Coroutine.h
	$(CXX) Coroutine.cpp -c $(CXXFLAGS)

example: Coroutine.h Coroutine.o
	$(CXX) example.cpp Coroutine.o $(CXXFLAGS) -o example

simple_scheduler_example: Coroutine.h Coroutine.o
	$(CXX) simple_scheduler_example.cpp Coroutine.o $(CXXFLAGS) -o simple_scheduler_example


clean:
	rm ./*.o
	rm example
	rm simple_scheduler_example

