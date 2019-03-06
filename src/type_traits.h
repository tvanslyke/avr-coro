#ifndef TIM_TYPE_TRAITS_H
#define TIM_TYPE_TRAITS_H

namespace tim::traits {

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
struct enable_if {};

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


} /* namespace tim::traits */

#endif /* TIM_TYPE_TRAITS_H */
