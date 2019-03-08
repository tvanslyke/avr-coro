/**
 * Copyright 2019 Timothy J. VanSlyke
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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
