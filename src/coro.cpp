#include "coro.h"
#include <Arduino.h>
#include <setjmp.h>
#include <stdint.h>

namespace ino::coro {

namespace detail {
jmp_buf* current_coro_context = &Coroutine<main_coro>::context;
} /* namespace detail */

[[noreturn]]
void coro_start(
	uint16_t stack_ptr, // r24/r25  r20/r21
	uint16_t start_fn   // r22/r23  r18/r19
) {
	asm volatile(
		"mov r30, r22;\n"\
		"mov r31, r23;\n"\
		"out __SP_L__, r24;\n"\
		"out __SP_H__, r25;\n"\
		"icall; "
		: /* no outputs */
		: /* no inputs */
		: "r30", "r31", "__SP_L__", "__SP_H__", "memory"
	);
}

} /* namespace ino::coro */

