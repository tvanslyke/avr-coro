/**
 * Copyright 2019 Timothy J. VanSlyke
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "Coroutine.h"
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
	// Set the stack pointer to the given value and call the start function.
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

Coroutine Coroutine::main = Coroutine{nullptr};
Coroutine* Coroutine::currently_running = &Coroutine::main;

bool Coroutine::is_suspended() const {
	return Coroutine::currently_running != this;
}

bool Coroutine::is_running() const {
	// return (not is_suspended()) and context_;
	return (not is_suspended());
}

bool Coroutine::is_done() const {
	return (this->context_ == nullptr) and (this != &Coroutine::main);
}

YieldResult yield_fast_to(Coroutine& coro) {
	assert(Coroutine::currently_running != &coro and "Cannot call yield_fast_to() on the currently-running coroutine.");
	jmp_buf context;
	auto save = Coroutine::currently_running;
	Coroutine::currently_running->context_ = &context;
	switch(setjmp(context)) {
	default:
		assert("Bad yield result");
	case static_cast<int>(YieldResult::Continue):
		return YieldResult::Continue;
	case static_cast<int>(YieldResult::Terminate):
		return YieldResult::Terminate;
	case static_cast<int>(YieldResult::Terminated):
		return YieldResult::Terminated;
	case 0: {
		auto* resume_context = coro.context_;
		coro.context_ = &context;
		longjmp(*(resume_context), static_cast<int>(YieldResult::Continue));
	}
	}
}

YieldResult yield_to(Coroutine& coro) {
	if(&coro == Coroutine::currently_running) {
		return YieldResult::Continue;
	}
	return yield_fast_to(coro);
}

void terminate(Coroutine& coro) {
	assert(&coro != Coroutine::currently_running);
	jmp_buf context;
	auto save = Coroutine::currently_running;
	Coroutine::currently_running->context_ = &context;
	switch(setjmp(context)) {
	default:
		assert(!"Bad terminate() call.  Coroutine ignored termination request.");
	case static_cast<int>(YieldResult::Terminated):
		break;
	case 0: {
		auto* term_ctx = coro.context_;
		coro.context_ = &context;
		longjmp(*term_ctx, static_cast<int>(YieldResult::Terminate));
	}
	}
}


} /* namespace ino::coro */

