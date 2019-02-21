#include "coro.h"
#include <setjmp.h>
#include <stdint.h>

namespace tim::coro {

namespace detail {

[[noreturn]]
void coro_start(
	uint16_t coroutine_addr, // r24/r25  - Already in correct registers
	uint16_t callable_addr,  // r22/r23  - Already in correct registers
	uint16_t stack_ptr,      // r20/r21  - Assign to __SP_L__ and __SP_H__ 
	uint16_t start_fn        // r18/r19  - Assign to Z register (r30/r31)
) {
	asm volatile(
		"mov r30, r18;\n"\
		"mov r31, r19;\n"\
		"out __SP_L__, r20;\n"\
		"out __SP_H__, r21;\n"\
		"icall;"
		: /* no outputs */
		: /* no inputs */
		: "r30", "r31", "__SP_L__", "__SP_H__"
	);
}

} /* namespace detail */

Coroutine Coroutine::main = Coroutine{};
Coroutine* Coroutine::currently_running = &Coroutine::main;

bool Coroutine::is_suspended() const {
	return Coroutine::currently_running != this;
}

bool Coroutine::is_running() const {
	return (not is_suspended()) and context_;
}

bool Coroutine::done() const {
	return (this->context_ == nullptr) and (this != &Coroutine::main);
}

void yield_fast_to(Coroutine& coro) {
	assert(Coroutine::currently_running != &coro && "Cannot call yield_fast_to() on the currently-running coroutine.");
	jmp_buf context;
	auto save = Coroutine::currently_running;
	Coroutine::currently_running->context_ = &context;
	if(not setjmp(context)) {
		longjmp(*(coro.context_), 1);
	} else {
		Coroutine::currently_running = save;
	}
}

void yield_to(Coroutine& coro) {
	if(&coro == Coroutine::currently_running) {
		return;
	}
	yield_fast_to(coro);
}

} /* namespace ino::coro */

