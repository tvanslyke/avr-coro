#include <csetjmp>

struct CoroutineRegisters {
	std::uint16_t program_counter;
	std::uint16_t stack_pointer;
	std::uint8_t status_register;
	std::uint8_t registers[32];
};

struct Coroutine {
	std::jmp_buf context;
	char* stack = nullptr;
	void (*start_function)(Coroutine*, const Coroutine*);

	static Coroutine main_coro;

	template <auto OtherFunc>
	void start(const Coroutine<OtherFunc>& resuming_coro) {
		if(not setjmp(this->context)) {
			/* Suspending */
			asm("lds %0, sp;" : : "g"(
			longjmp(other->context);
		}
	}

	template <auto OtherFunc>
	void yield_to(const Coroutine<OtherSize, OtherFunc>& other) {
		if(not setjmp(this->context)) {
			/* Suspending */
			longjmp(other->context);
		}
	}
}
Coroutine Coroutine::main_coro {
	{},
	nullptr,
	nullptr
};

void loop_coro(Coroutine* self, const Coroutine* caller) {
	Serial.println("Loop coro!");
	self->yield_to(caller);
}




