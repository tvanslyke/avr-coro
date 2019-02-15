#ifndef INO_CORO_H
#define INO_CORO_H

#include <cstddef>
#include <stdint.h>
#include <setjmp.h>

namespace ino::coro {

namespace detail {

template <class T, class U>
struct is_same {
	static constexpr bool value = false;
};

template <class T>
struct is_same<T, T> {
	static constexpr bool value = false;
};

template <class T, class U>
inline constexpr bool is_same_v = is_same<T, U>::value;

template <bool B, class T>
struct enable_if;

template <class T>
struct enable_if<false, T> {
	
};

template <class T>
struct enable_if<true, T> {
	using type = T;
};

template <bool B, class T>
using enable_if_t = typename enable_if<B, T>::type;

} /* namespace detail */


[[noreturn]]
void coro_start(uint16_t stack_ptr, uint16_t start_fn);

template <auto Coro>
struct coroutine_traits {
	static constexpr std::size_t stack_size = 128u;
	using argument_type = void;
};

template <auto Coro>
inline constexpr std::size_t coroutine_stack_size = 128u;

template <auto Coro>
struct Coroutine {
	static char stack[coroutine_stack_size<Coro>];
	static jmp_buf context;

	Coroutine() = delete;
};

template <auto Coro>
char Coroutine<Coro>::stack[coroutine_stack_size<Coro>] = {0};

template <auto Coro>
jmp_buf Coroutine<Coro>::context;

/* Never actually invoked. */
void main_coro();

template <>
inline constexpr std::size_t coroutine_stack_size<main_coro> = 1u;

namespace detail {

extern jmp_buf* current_coro_context;

} /* namespace detail */


template <auto Coro>
void start() {
	if(not setjmp(*detail::current_coro_context)) {
		static_assert(sizeof(void*) == 2u);
		detail::current_coro_context = &Coroutine<Coro>::context;
		coro_start(
			reinterpret_cast<uint16_t>(&Coroutine<Coro>::stack[coroutine_stack_size<Coro> - 1u]),
			reinterpret_cast<uint16_t>(Coro)
		);
		__builtin_unreachable();
	}
}

template <auto Coro>
void yield_to() {
	using traits = coroutine_traits<Coro>;
	if constexpr(detail::is_same_v<
		if(not setjmp(*detail::current_coro_context)) {
			detail::current_coro_context = &Coroutine<Coro>::context;
			/* Suspending */
			longjmp(Coroutine<Coro>::context, 1);
			__builtin_unreachable();
		}
	} else {
		
	]
}

} /* namespace ino::coro */

#endif /* INO_CORO_H */
