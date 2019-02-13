CC=avr-gcc-8
CXX=avr-g++-8
AR=avr-ar

DEVICE=__AVR_ATmega328P__

CXXFLAGS=-mmcu=atmega328p -std=c++17 -O2 -fmax-errors=5 -Wall -Wextra -DF_CPU=16000000L -DARDUINO=105 -ffunction-sections -fdata-sections -w -I./../arduino/ -I./ArduinoSTL/src

OBJECTS=main.o coro.o

firmware.elf: $(OBJECTS) ./../arduino/libarduino.a
	$(CXX) $(OBJECTS) ./../arduino/libarduino.a $(CXXFLAGS) -o firmware.elf

main.o: main.cpp coro.h
	$(CXX) main.cpp -c $(CXXFLAGS) 

coro.o: coro.cpp coro.h
	$(CXX) coro.cpp -c $(CXXFLAGS) 

coro_asm:
	$(CXX) coro.cpp -c -S -o - $(CXXFLAGS) | c++filt | grep -vE '\s+\.'



./../arduino/libarduino.a:
	cd ./../arduino && $(MAKE)

clean:
	rm ./*.o
	rm firmware.elf

