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

