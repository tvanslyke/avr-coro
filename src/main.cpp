#include <Arduino.h>
#include "coro.h"
#include <cstddef>

template <std::size_t Duration, int Pin>
void blinky_coro(Coroutine* self, Coroutine* caller) {
	Serial.print("Starting 'blinky_coro<");
	Serial.print(Duration);
	Serial.print(", ");
	Serial.print(Pin);
	Serial.println(">()'.");
	pinMode(Pin, OUTPUT);
	int logic_level = LOW;
	self->yield(caller);
	digitalWrite(Pin, logic_level);
	for(;;) {
		auto start = millis();
		auto now = start;
		while(static_cast<std::size_t>(now - start) < Duration) {
			self->yield_to(caller);
			now = millis();
		}
		switch(logic_level) {
		case LOW:
			logic_level = HIGH;
			break;
		case HIGH:
			logic_level = LOW;
			break;
		}
		digitalWrite(Pin, logic_level);
	}
}

static Coroutine* coroutines[9] = {nullptr};

void setup() {
	Serial.begin(115200);
	Serial.println("Starting...");
	Serial.flush();
	coroutines[0] = ino::coro::start_coroutine<blinky_coro<   50,  2>>();
	coroutines[1] = ino::coro::start_coroutine<blinky_coro<  100,  3>>();
	coroutines[2] = ino::coro::start_coroutine<blinky_coro<  200,  4>>();
	coroutines[3] = ino::coro::start_coroutine<blinky_coro<  400,  5>>();
	coroutines[4] = ino::coro::start_coroutine<blinky_coro<  800,  6>>();
	coroutines[5] = ino::coro::start_coroutine<blinky_coro< 1600,  7>>();
	coroutines[6] = ino::coro::start_coroutine<blinky_coro< 3200,  8>>();
	coroutines[7] = ino::coro::start_coroutine<blinky_coro< 6400,  9>>();
	coroutines[8] = ino::coro::start_coroutine<blinky_coro<12800, 10>>();
}

void loop() {
	for(;;) {
		auto start = millis();
		for(Coroutine* coro: coroutines) {
			ino::coro::resume_coro(coro);
		}
		auto stop = millis();
		Serial.print("Executed ");
		Serial.print(sizeof(coroutines) / sizeof(coroutines[0]));
		Serial.print(" coroutines in ");
		Serial.print(stop - start);
		Serial.println(" milliseconds.");
	}
}

int main(void)
{
	init();

#if defined(USBCON)
	USBDevice.attach();
#endif
	
	setup();
	Serial.println("setup done!");
    
	for (;;) {
		loop();
		if (serialEventRun) serialEventRun();
	}
        
	return 0;
}

