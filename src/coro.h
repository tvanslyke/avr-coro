#ifndef INO_CORO_H
#define INO_CORO_H

#include <cstddef>
#include <stdint.h>
#include <setjmp.h>

namespace ino::coro {

struct Coroutine {

	static Coroutine main_coroutine;

	void yield_to(Coroutine* continuation);

private:

	Coroutine() = default;

	void start(void (*start_function)(Coroutine*, Coroutine*), char* stack, Coroutine* caller);
	
	template <void (*Coro)(Coroutine*, Coroutine*)>
	friend Coroutine* start_coroutine();

	jmp_buf context;
};

template <void (*Coro)(Coroutine*, Coroutine*)>
struct coroutine_traits {
	static constexpr std::size_t stack_size = 128u;
};

template <void (*StartFn)(Coroutine*, Coroutine*)>
Coroutine* start_coroutine() {
	using traits = coroutine_traits<StartFn>;
	static_assert(traits::stack_size >= 1, "Coroutine stack size cannot be zero!");
	static char coroutine_stack[traits::stack_size];
	static Coroutine coro;
	coro.start(StartFn, &coroutine_stack[traits::stack_size - 1u], &Coroutine::main_coroutine);
	return &coro;
}


template <void (*Loop)(), void (*Setup)() = nullptr>
void looped_coroutine(Coroutine* self, Coroutine* caller) {
	static_assert(Loop != nullptr);
	if constexpr(Setup) {
		Setup();
	}
	for(;;) {
		self->yield_to(caller);
		Loop();
	}
}

inline void resume_coro(Coroutine* coro) {
	Coroutine::main_coroutine.yield_to(coro);
}

} /* namespace ino::coro */

#endif /* INO_CORO_H */
