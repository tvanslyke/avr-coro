#include "coro.h"
#include <stddef.h>

using namespace tim::coro;

template <size_t N>
struct RoundRobinScheduler {

	/** Initialize the scheduler with all of the tasks it will be scheduling */
	template <class ... Tasks>
	RoundRobinScheduler(Tasks* ... tasks):
		tasks_{tasks ...}
	{
		static_assert(sizeof...(Tasks) == N);
	}

	/** Start the scheduler. */
	void begin() {
		// Move any null tasks to the end of the task list.
		this->partition();
		// Start all of the coroutines.
		for(Coroutine* coro: tasks_) {
			if(not coro) {
				break;
			}
			coro->begin();
		}
		// Run the round-robin scheduler routine.
		for(;;) {
			if(not tasks_[0u]) {
				return;
			}
			for(size_t i = 0u; i < N; ++i) {
				auto* coro = tasks_[i];
				if(not coro) {
					break;
				}
				switch(yield_to(*coro)) {
				default:
				case YieldResult::Continue:
					break;
				case YieldResult::Terminated:
					// Coroutine terminated, remove it.
					remove_task(i);
					[[fallthrough]]
				case YieldResult::Terminate:
					// Coroutine requested termination; return.
					return;
				}
			}
		}
	}

	/** Terminate all remaining tasks. */
	void end() {
		size_t i = 0u;
		for(auto* coro: tasks_) {
			if(not coro) {
				return;
			}
			terminate(*coro);
			remove_task(i);
			++i;
		}
	}

private:

	/** Move all of the non-null tasks to the beginning. */
	void partition() {
		size_t dest_pos = 0u;
		size_t next = 0;
		// find the first null task.
		for(; dest_pos < N; ++dest_pos) {
			if(not tasks_[dest_pos]) {
				break;
			}
		}
		// find the first non-null task after that.
		for(next = dest_pos; next < N; ++next) {
			if(tasks_[next]) {
				break;
			}
		}
		// move all the nulls to the end.
		for(; next < N; ++next) {
			if(tasks_[next]) {
				tasks_[dest_pos++] = tasks_[next];
				tasks_[next] = nullptr;
			}
		}
	}

	/** Remove a coroutine from the list of to-be-scheduled tasks. */
	void remove_task(size_t index) {
		if(index >= N) {
			return;
		}
		tasks_[index] = nullptr;
		for(size_t i = index + 1u; i < N; ++i) {
			if(not tasks_[i]) {
				break;
			}
			tasks_[i - 1u] = tasks_[i];
		}
	}

	Coroutine* tasks_[N] = {nullptr};
};

template <class ... Tasks>
RoundRobinScheduler(Tasks* ...) -> RoundRobinScheduler<(sizeof...(Tasks))>;

// Globals are bad, but this is an example.
static volatile unsigned long fib_value = 0u;
// Fibonacci generator coroutine.
static void generate_fib_numbers(Coroutine& self) {
	fib_value = 1u;
	yield_to(Coroutine::main);
	fib_value = 1u;
	yield_to(Coroutine::main);
	unsigned long prev = 1u;
	unsigned long curr = 1u;
	for(;;) {
		auto save = curr;
		curr = prev + curr;
		prev = save;
		fib_value = curr;
		yield_to(Coroutine::main);
	}
}
// Actual coroutine object.
static auto fib_generator = BasicCoroutine{generate_fib_numbers};

// Fibonacci printer coroutine.
static void print_fib_numbers(Coroutine& self) {
	for(unsigned long i = 0u;; ++i) {
		printf("fib(%lu) = %lu\n", i, fib_value);
		yield_to(Coroutine::main);
	}
}
// Actual coroutine object.
static auto fib_printer = BasicCoroutine{print_fib_numbers};

// Set up the scheduler so that the generator runs and then the printer runs.
static auto scheduler = RoundRobinScheduler(&fib_generator, &fib_printer);

int main () {
	// Print the Fibonacci numbers in order.
	scheduler.begin();
}

