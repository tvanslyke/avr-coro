#include "coro.h"
#include <Arduino.h>
#include <setjmp.h>
#include <stdint.h>

namespace ino {

[[gnu::noinline, noreturn]]
void coro_start(
	uint16_t callee,    // r24/r25
	uint16_t caller,    // r22/r23
	uint16_t stack_ptr, // r20/r21
	uint16_t start_fn   // r18/r19
) {
	asm volatile(
		"mov r30, r18;\n"\
		"mov r31, r19;\n"\
		"out __SP_L__, r20;\n"\
		"out __SP_H__, r21;\n"\
		"icall; "
		: /* no outputs */
		: /* no inputs */
		: "r30", "r31", "__SP_L__", "__SP_H__", "memory"
	);
}

void Coroutine::start(void (*start_function)(Coroutine*, Coroutine*), char* stack, Coroutine* caller) {
	assert(not this->caller && "Attempt to restart already-active coroutine.");
	if(not setjmp(caller->context)) {
		/* Suspending this, starting callee */
		static_assert(sizeof(this) == 2);
		coro_start(
			reinterpret_cast<uint16_t>(this),
			reinterpret_cast<uint16_t>(caller),
			reinterpret_cast<uint16_t>(stack),
			reinterpret_cast<uint16_t>(start_function)
		);
		__builtin_unreachable();
	}
}

void Coroutine::yield_to(Coroutine* continuation) {
	if(not setjmp(this->context)) {
		Serial.println("yielding!");
		Serial.flush();
		delay(1000);
		/* Suspending */
		longjmp(continuation->context, static_cast<int>(this));
	}
}

Coroutine Coroutine::main_coro = Coroutine();

} /* namespace ino */

