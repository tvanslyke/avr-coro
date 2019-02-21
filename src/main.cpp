#include <Arduino.h>
#include <cstddef>
#include "coro.h"

using namespace tim::coro;

template <int Pin>
void button_press_coro(Coroutine& self) {
	Serial.print("Stating button_press_coro<");
	Serial.print(Pin);
	Serial.println(">().");
	Serial.flush();
	pinMode(Pin, INPUT);
	yield_to(Coroutine::main);
	for(;;) {
		if(digitalRead(Pin) == LOW) {
			auto start = micros();
			do {
				yield_to(Coroutine::main);
			} while(digitalRead(Pin) == LOW);
			auto duration = micros() - start;
			Serial.print("button_press_coro<");
			Serial.print(Pin);
			Serial.print(">():");
			Serial.print("Button pressed for ");
			Serial.print(duration);
			Serial.println(" microseconds.");
		}
		yield_to(Coroutine::main);
	}
}

UniqueCoroutine<void (*)(Coroutine&), 128u> pin2_button_coro = UniqueCoroutine{button_press_coro<2>};
UniqueCoroutine<void (*)(Coroutine&), 128u> pin3_button_coro = UniqueCoroutine{button_press_coro<3>};

void setup() {
	Serial.begin(115200);
	Serial.println("Initializing...");
	Serial.flush();
	pinMode(13, OUTPUT);
	digitalWrite(13, LOW);
	pin2_button_coro.start();
	pin3_button_coro.start();
}

void loop() {
	Serial.println("Starting loop()");
	Serial.flush();

	for(;;) {
		auto start = micros();
		digitalWrite(13, HIGH);
		while(micros() - start < 500000ul) {
			yield_to(pin2_button_coro);
			yield_to(pin3_button_coro);
		}
		start = micros();
		digitalWrite(13, LOW);
		while(micros() - start < 500000ul) {
			yield_to(pin2_button_coro);
			yield_to(pin3_button_coro);
		}
	}
}

int main(void)
{
	init();

#if defined(USBCON)
	USBDevice.attach();
#endif
	
	setup();
 	
	for (;;) {
		loop();
		if (serialEventRun) serialEventRun();
	}
        
	return 0;
}

