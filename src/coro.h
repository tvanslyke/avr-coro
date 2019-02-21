#ifndef INO_CORO_H
#define INO_CORO_H

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include "assert.h"

namespace tim::coro {

struct Coroutine;

void yield_fast_to(Coroutine& coro);

void yield_to(Coroutine& coro);

void yield();

namespace detail {

template <class T, class U>
struct is_same {
	static constexpr bool value = false;
};

template <class T>
struct is_same<T, T> {
	static constexpr bool value = true;
};

template <class T, class U>
inline constexpr bool is_same_v = is_same<T, U>::value;

template <bool B, class T>
struct enable_if {

};

template <class T>
struct enable_if<true, T> {
	using type = T;
};

template <class T>
struct add_rvalue_reference {
	using type = T&&;
};

template <class T>
struct add_rvalue_reference<T&> {
	using type = T&;
};

template <class T>
struct add_rvalue_reference<T&&> {
	using type = T&&;
};

template <class T>
typename add_rvalue_reference<T>::type declval() noexcept;

[[noreturn]]
void coro_start(uint16_t context, uint16_t stack_ptr, uint16_t start_fn);

} /* namespace detail */

namespace detail {

template <class Callable>
void start_coroutine(Coroutine* caller, Callable* callable);


[[noreturn]]
void coro_start(
	uint16_t coroutine_addr,
	uint16_t callable_addr,
	uint16_t stack_ptr,
	uint16_t start_fn
);

} /* namespace detail */

struct Coroutine {

	static Coroutine main;

	Coroutine(const Coroutine&) = delete;
	Coroutine(Coroutine&&) = delete;

	Coroutine& operator=(const Coroutine&) = delete;
	Coroutine& operator=(Coroutine&&) = delete;

	bool is_running() const;
	bool is_suspended() const;
	bool done() const;

protected:
	Coroutine() = default;

	static Coroutine* currently_running;

	template <class Callable>
	void initialize(Callable& callable, char* stack_ptr) {
		assert((not this->context_) and "Attempt to start an already-started coroutine!");
		jmp_buf context;
		Coroutine* caller = Coroutine::currently_running;
		Coroutine::currently_running = this;
		caller->context_ = &context;
		if(not setjmp(context)) {
			detail::coro_start(
				reinterpret_cast<uint16_t>(caller),
				reinterpret_cast<uint16_t>(&callable),
				reinterpret_cast<uint16_t>(stack_ptr),
				reinterpret_cast<uint16_t>(detail::start_coroutine<Callable>)
			);
		} else {
			Coroutine::currently_running = caller;
		}
	}

private:

	template <class Callable>
	friend void detail::start_coroutine(Coroutine* coro, Callable* callable);

	friend void yield();
	friend void yield_fast_to(Coroutine&);
	friend void yield_to(Coroutine&);

protected:
	jmp_buf* context_ = nullptr;
};


template <size_t N>
struct stack_size {
	static constexpr size_t value = N;
};

namespace detail {

template <class Callable>
void start_coroutine(Coroutine* caller, Callable* callable) {
	{
		volatile char stack_byte;
		Serial.print("start_coroutine(");
		Serial.print(reinterpret_cast<uint16_t>(Coroutine::currently_running), HEX);
		Serial.print(", ");
		Serial.print(reinterpret_cast<uint16_t>(callable));
		Serial.print(") with stack address ");
		Serial.println(reinterpret_cast<uint16_t>(&stack_byte));
		Serial.flush();
	}
	assert(Coroutine::currently_running);
	assert(Coroutine::currently_running != caller);
	yield_to(*caller);
	(*callable)(*Coroutine::currently_running);
	jmp_buf* caller_ctx = Coroutine::currently_running->context_;
	assert(caller_ctx and "Coroutine returned with no caller context to yield to.");
	Coroutine::currently_running->context_ = nullptr;
	longjmp(*caller_ctx, 1);
}

} /* namespace detail */

template <size_t N>
using stack_size_t = stack_size<N>;

template <size_t N>
inline constexpr auto stack_size_v = stack_size_t<N>{};

template <class Callable, size_t StackSz>
struct UniqueCoroutine: Coroutine {
	static_assert(
		detail::is_same_v<void, decltype(detail::declval<Callable&>()(detail::declval<Coroutine&>()))>,
		"UniqueCoroutine callable object must have signature 'void(Coroutine&)'"
	);
	static_assert(StackSz != 0u, "Stack size cannot be zero for UniqueCoroutine.");

	UniqueCoroutine(Callable callable):
		UniqueCoroutine(callable, stack_size_v<StackSz>)
	{
		
	}

	UniqueCoroutine(Callable callable, stack_size_t<StackSz>):
		Coroutine(),
		callable_(callable),
		stack_{0}
	{

	}

	void start() {
		initialize(this->callable_, &stack_[StackSz - 1u]);
	}

private:
	Callable callable_;
	char stack_[StackSz];
};

template <class Callable, size_t StackSz>
UniqueCoroutine(Callable, stack_size_t<StackSz>) -> UniqueCoroutine<Callable, StackSz>;

template <class Callable>
UniqueCoroutine(Callable) -> UniqueCoroutine<Callable, 128u>;

namespace detail {

template <auto Fn>
struct CoroutineDefinition {
	static UniqueCoroutine<decltype(Fn), 128u> value;
};

template <auto Fn>
UniqueCoroutine<decltype(Fn), 128u> CoroutineDefinition<Fn>::value = UniqueCoroutine(Fn);

} /* namespace detail */


} /* namespace tim::coro */

#endif /* INO_CORO_H */
