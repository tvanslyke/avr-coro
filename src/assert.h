/**
 * Copyright 2019 Timothy J. VanSlyke
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef TIM_ASSERT_H
#define TIM_ASSERT_H

#include <stdio.h>

namespace tim::detail {

[[noreturn]]
inline void assert_fail(const char* assertion, const char* file, long linenumber) {
	printf(file);
	printf(" line ");
	printf("%ld", linenumber);
	printf("\n");
	printf("Assertion: \"");
	printf(assertion);
	printf("\" failed.");
	volatile unsigned n = 0;
	for(;;) {
		++n;
	}
}

} /* namespace tim::detail */

#ifndef assert
# ifdef defined(NDEBUG) || defined(TIM_CORO_NO_ASSERT)
#  define assert(x) (void)(x)
# else
#  define assert(x) \
         (x) ? (void)0 : tim::detail::assert_fail(#x, __FILE__, __LINE__)
# endif
#endif
	
#endif /* TIM_ASSERT_H */
