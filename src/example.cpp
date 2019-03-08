#include "Coroutine.h"
#include <stdio.h>

extern tim::coro::Coroutine& coro1;
extern tim::coro::Coroutine& coro2;
extern tim::coro::Coroutine& coro3;

inline auto c1 = tim::coro::BasicCoroutine{
	[](tim::coro::Coroutine& self) -> void {
		for(long i = 0; ; ++i) {
			printf("coro1: %ld", i);
			if(yield_to(coro2) == tim::coro::YieldResult::Terminate) {
				return;
			}
		}
	}
};
tim::coro::Coroutine& coro1 = c1;

inline auto c2 = tim::coro::BasicCoroutine(
	[](tim::coro::Coroutine&) {
		for(long i = 0; ; ++i) {
			printf("coro2: %ld", i);
			if(yield_to(coro3) == tim::coro::YieldResult::Terminate) {
				return;
			}
		}
	}
);
tim::coro::Coroutine& coro2 = c2;

inline auto c3 = tim::coro::BasicCoroutine(
	[](tim::coro::Coroutine& self) {
		for(long i = 0; ; ++i) {
			printf("coro3: %ld", i);
			if(yield_to(coro1) == tim::coro::YieldResult::Terminate) {
				return;
			}
		}
	}
);
tim::coro::Coroutine& coro3 = c3;

int main() {
	c1.begin();
	c2.begin();
	c3.begin();
	yield_to(coro1);
}

