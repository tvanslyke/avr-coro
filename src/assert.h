#ifndef TIM_ASSERT_H
#define TIM_ASSERT_H

#include <Arduino.h>

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
