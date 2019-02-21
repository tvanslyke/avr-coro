#ifndef INO_ASSERT_H
#define INO_ASSERT_H

#include <Arduino.h>

namespace ino::detail {

[[noreturn]]
inline void assert_fail(const char* assertion, const char* file, long linenumber) {
	Serial.print(file);
	Serial.print(" line ");
	Serial.println(linenumber);
	Serial.print("Assertion: \"");
	Serial.print(assertion);
	Serial.println("\" failed.");
	volatile unsigned n = 0;
	for(;;) {
		++n;
	}
}

} /* namespace ino::detail */

#ifdef NDEBUG
# define assert(x) (void)(x)
#else
# define assert(x) \
	(x) ? (void)0 : ino::detail::assert_fail(#x, __FILE__, __LINE__)
#endif
	
#define UNREACHABLE() \
	ASSERT(!"Unreachable code path.")

#endif /* INO_ASSERT_H */
