/**
 * Copyright 2019 Timothy J. VanSlyke
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef TIM_CORO_COROUTINE_H
#define TIM_CORO_COROUTINE_H

#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include "assert.h"
#include "type_traits.h"

namespace tim::coro {

enum class YieldResult: int {
	// Indicates that the resumed coroutine should continue running.
	Continue=1,
	// Indicates 
	Terminate=2,
	Terminated=3
};

struct Coroutine;

/**
 * Yield execution of the currently-running coroutine to the given coroutine.
 * It is NOT safe for a coroutine to use this function to yield to itself.
 */
[[nodiscard]]
YieldResult yield_fast_to(Coroutine& coro);

/**
 * Yield execution of the currently-running coroutine to the given coroutine.
 * It is safe for a coroutine to use this function to yield to itself.
 */
[[nodiscard]]
YieldResult yield_to(Coroutine& coro);

/**
 * Send a 'Terminate' signal to the coroutine.  After calling
 * terminate on a coroutine object, the coroutine must be 'start()'ed
 * again before resuming it.
 */
void terminate(Coroutine& coro);

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

	Coroutine() = delete;

	Coroutine(const Coroutine&) = delete;
	Coroutine(Coroutine&&) = delete;

	Coroutine& operator=(const Coroutine&) = delete;
	Coroutine& operator=(Coroutine&&) = delete;

	/** True if this coroutine is the currently-running coroutine. */
	bool is_running() const;
	/** True if this coroutine is NOT the currently-running coroutine. */
	bool is_suspended() const;
	/** True if this coroutine must be started before resuming. */
	bool is_done() const;

	/** Terminate the coroutine if it is running. */
	void end() { terminate(*this); }

	/**
	 * Start the coroutine.
	 * Initializes the coroutine such that other coroutines can
	 * begin yielding to this coroutine object.  The actual callable
	 * object is not invoked until the first time this coroutine
	 * is resumed.
	 */
	void begin() {
		assert(this->start_fn_);
		this->start_fn_(*this);
	}
protected:

	Coroutine(void (*start_fn)(Coroutine&)):
		start_fn_(start_fn),
		context_(nullptr)
	{
		
	}

	static Coroutine* currently_running;

	/**
	 * Set up all the necessary machinery to allow other coroutines
	 * to start yielding to this one.
	 */
	template <class Callable>
	void initialize(Callable& callable, char* stack_ptr) {
		assert((not this->context_) and "Attempt to start an already-started coroutine!");
		// This jmp_buf will be used to allow the coroutine we're starting to jump back
		// to this execution context once it's started.
		jmp_buf context;
		// Save the currently-running coroutine.
		Coroutine* caller = Coroutine::currently_running;
		// Save the current context to the currently-running coroutines context_ pointer.
		caller->context_ = &context;
		// Make this coroutine the currently-running coroutine and jump to it.
		Coroutine::currently_running = this;
		if(not setjmp(context)) {
			// Start the coroutine.
			detail::coro_start(
				reinterpret_cast<uint16_t>(caller),
				reinterpret_cast<uint16_t>(&callable),
				reinterpret_cast<uint16_t>(stack_ptr),
				reinterpret_cast<uint16_t>(detail::start_coroutine<Callable>)
			);
		} else {
			// All done, this coroutine is now resumable.
			Coroutine::currently_running = caller;
		}
	}

	template <class Callable>
	friend void detail::start_coroutine(Coroutine* coro, Callable* callable);

	friend [[nodiscard]] YieldResult yield_fast_to(Coroutine&);
	friend [[nodiscard]] YieldResult yield_to(Coroutine&);
	friend void terminate(Coroutine&);

protected:
	/**
	 * Pointer to function that starts the coroutine.
	 */
	void (*start_fn_)(Coroutine&);
	/**
	 * If this coroutine is the currently-running coroutine, then 'context_' is the
	 * context of whoever most-recently resumed this coroutine.
	 * Otherwise 'context_' holds the context needed to resume the coroutine.
	 */
	jmp_buf* context_ = nullptr;
};


namespace detail {

/**
 *  Entry point for all coroutines besides main.
 */
template <class Callable>
void start_coroutine(Coroutine* caller, Callable* callable) {
	assert(Coroutine::currently_running);
	assert(Coroutine::currently_running != caller);
	// Immediately yield to the caller.  The first time this coroutine
	// is resumed, we jump right into the callable.
	switch(yield_to(*caller)) {
	case YieldResult::Continue:
		// Start the actual coroutine.
		(*callable)(*Coroutine::currently_running);
		[[fallthrough]]
	case YieldResult::Terminate:
	default:
		(void)0;
	}
	// The coroutine has finished executing, clean up and then jump to the caller.
	// Note that the caller in this case is whomever lsat resumed this coroutine.  
	jmp_buf* caller_ctx = Coroutine::currently_running->context_;
	assert(caller_ctx and "Coroutine terminated with no caller context to yield to.");
	Coroutine::currently_running->context_ = nullptr;
	// Jump back to whomever last resumed this coroutine.
	longjmp(*caller_ctx, static_cast<int>(YieldResult::Terminated));
}

} /* namespace detail */

/**
 * Tag type used to specify static stack size for a coroutine.
 */
template <size_t N>
struct stack_size {
	static constexpr size_t value = N;
};

template <size_t N>
inline constexpr auto stack_size_v = stack_size<N>{};

/**
 * Coroutine type that invokes an object of type 'Callable' with
 * a stack of 'StackSz' bytes.
 */
template <class Callable, size_t StackSz>
struct BasicCoroutine: Coroutine {
	static_assert(
		traits::is_same_v<void, decltype(traits::declval<Callable&>()(traits::declval<Coroutine&>()))>,
		"BasicCoroutine callable object must have signature 'void(Coroutine&)'"
	);
	static_assert(StackSz != 0u, "Stack size cannot be zero for BasicCoroutine.");

	/**
	 * Create a BasicCoroutine instance from the object of type 'Callable'.
	 */
	BasicCoroutine(Callable callable):
		BasicCoroutine(callable, stack_size_v<StackSz>)
	{
		
	}

	/**
	 * Constructor to support deducing StackSz with CTAD deduction guides.
	 */
	BasicCoroutine(Callable callable, stack_size<StackSz>):
		Coroutine(BasicCoroutine::start_function),
		callable_(callable),
		stack_{0}
	{
		
	}

	~BasicCoroutine() {
		end();
	}


private:
	void start_coroutine() {
		assert(this->context_);
		this->initialize(callable_, &stack_[StackSz - 1u]);
	}

	static void start_function(Coroutine& self) {
		static_cast<BasicCoroutine&>(self).start_coroutine();
	}

	// Callable object that is invoked upon starting.
	Callable callable_;
	// Call stack for this coroutine.
	char stack_[StackSz];
};


/** Deduction guides for BasicCoroutine. */

template <class Callable, size_t StackSz>
BasicCoroutine(const Callable&, stack_size<StackSz>) -> BasicCoroutine<Callable, StackSz>;

template <class Callable>
BasicCoroutine(const Callable&) -> BasicCoroutine<Callable, 128u>;

template <size_t StackSz>
BasicCoroutine(void (Coroutine&), stack_size<StackSz>) -> BasicCoroutine<void (*)(Coroutine&), StackSz>;

BasicCoroutine(void (Coroutine&)) -> BasicCoroutine<void (*)(Coroutine&), 128u>;

} /* namespace tim::coro */

#endif /* INO_CORO_COROUTINE_H */
