#include <Arduino.h>
#include "coro.h"
#include <cstddef>
#include <math.h>

static std::size_t pulse_width = 0u;

template <int Pin, std::size_t Period>
static void pwm_coro() {
	Serial.print("Starting pwm_coro<");
	Serial.print(Pin);
	Serial.print(", ");
	Serial.print(Period);
	Serial.println(">()");
	std::size_t pulse_start = micros();
	pinMode(Pin, OUTPUT);
	bool pulse_state = false;
	for(;;) {
		ino::coro::yield_to<ino::coro::main_coro>();
		auto now = micros();
		auto current_width = pulse_start - now;
		if(pulse_state) {
			if(current_width > pulse_width) {
				digitalWrite(Pin, LOW);
			}
		} else {
			pulse_start += Period;
			if(current_width >= Period) {
				digitalWrite(Pin, HIGH);
			}
		}
	}
}

void setup() {
	Serial.begin(115200);
	Serial.println("Initializing...");
	Serial.flush();
	pinMode(3, OUTPUT);
	// ino::coro::start<pwm_coro<4, 20000>>();
}

void loop() {
	constexpr auto period = 2000ul;
	constexpr auto pulse_max = 240;
	constexpr auto pulse_min = 60;
	auto start_time = millis();
	for(;;) {
		auto now = millis();
		pulse_width = 8u + ((now - start_time) * (22)) / period;
		Serial.println(pulse_width);
		analogWrite(3, pulse_width);
		if(now - start_time > 2000u) {
			start_time = now;
		}
		// auto t = micros();
		// ino::coro::yield_to<pwm_coro<4, 20000>>();
		// t = micros() - t;
		// Serial.print("Completed coroutine in ");
		// Serial.print(t);
		// Serial.println(" microseconds.");
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

