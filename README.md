Coroutines for cooperative multitasking in C++17 on AVR targets.
# Table of Contents
1. [Introduction](#introduction)
2. [Usage](#usave)
3. [Documentation](#documentation)
4. [Requirements](#requirements)
5. [License](#license)
6. [Author](#author)

# Introduction
This is a small and simple C++17 coroutine implementation for AVR.  This library works on Arduino but isn't meant to be strictly an "arduino library".  Note that this also has nothing to do with standard C++20 coroutines.

# Usage
This library optimizes for the use case where a static number of coroutines will be used (though it is possible to spawn new coroutines dynamically).  For example, a project may have one coroutine read from sensors, another control some motors according to the sensor readings, and another talking to a device over an I2C/two-wire interface.  Each of these tasks may have to do some "busy-waiting" at several points when, rather than spinning (like arduino's `delay()` function) the waiting task yields to other tasks that can do work in the mean time.  This pattern is fairly common in embedded systems and coroutines offer a workable solution.

## Examples
`src/example.cpp` and `src/simple_scheduler_example.cpp` show most of the functionality provided by the library.


## Static Library `libtimcoro.a`
The provided `libtimcoro.a` under `release/` is compiled from `Coroutine.cpp` using `avr-g++-8` with optimization level `-O2` and no debug information (but with assertions enabled).  This library can be linked with in place of adding `Coroutine.cpp` to your build.  To build `libtimcoro.a` with different compilers/parameters, `src/Makefile` should be modified as needed.

## Headers
The `Coroutine.h` header declares the types and functions provided by the library.  The `assert.h` and `type_traits.h` headers are private to the library but are included by `Coroutine.h`.

# Documentation

## Namespace `tim::coro`
All types and functions exposed by this library live in the namespace `tim::coro`.  The namespace `tim` is used because my first name is Tim and it makes for a nice three-letter top-level namespace to use in my C++ projects.

### Type `Coroutine`
The type `tim::coro::Coroutine` encapsulates a coroutine that can be resumed (by calling `yield_to(some_coroutine_object)`) and suspended (by calling `yield_to(some_other_coroutine_object)`).  `tim::coro::Coroutine` objects are not constructed by the user directly; instead users instantiate type `tim::coro::BasicCoroutine` (which is derived from `tim::coro::Coroutine`).

Notes:
* `tim::coro::Coroutine` has no virtual functions (and thus does not generate a vtable).
* `tim::coro::Coroutine` is neither copyable nor movable.

#### Member Function `bool Coroutine::is_running() const`
Return `true` if the coroutine is currently running or `false` if it is suspended or hasn't been started.

#### Member Function `bool Coroutine::is_suspended() const`
Return `true` if the coroutine is *not* currently running or `false` if it is currently running.

#### Member Function `bool Coroutine::is_done() const`
Return `true` if the coroutine is *not* currently running **and** it cannot be resumed/yielded-to.  If a `Coroutine` object returns `true` from this function, it must be started by calling `void Coroutine::begin()`.

#### Member Function `void Coroutine::begin()`
Initializes the coroutine object so that it becomes resumable.  Calling this function initializes the coroutine's context buffer (a `jmp_buf`, in practice) and initializes its call stack such that the next time it is resumed, the actual coroutine code will be invoked.

#### Member Function `void Coroutine::end()`
Sends a terminate "signal" to the coroutine object.  This will (provided the coroutine does not ignored the terminate signal) unwind the coroutine's stack (calling any destructors along the way), and return to the caller.  After this the coroutine is no longer resumable and must be started again by calling `Coroutine::begin()` before attempting to resume it.

### Enumeration `YieldResult`
The scoped enumeration `YieldResult` is returned from the `yield_*` free functions in namespace `tim::coro`.  Yield result has three possible values:
1. `Continue` - `YieldResult::Continue` is returned from one of the `yield_*` functions to indicate that the `yield_*` caller should resume executing normally.
    * When calling one of the `yield_*` functions to resume a coroutine object, that coroutine object will be resumed with the `YieldResult::Continue` signal.
2. `Terminated` - `YieldResult::Terminated` is returned from one of the `yield_*` functions to indicate that the `yield_*` caller may resume executing normally, but that the yielded-to coroutine has been ended/terminated and must be restarted again before yielding to it.
3. `Terminate` - `YieldResult::Terminate` is returned from one of the `yield_*` functions to indicate that the `yield_*` caller should terminate execution and with a `return` statement (after perhaps doing any non-automatic cleanup).  Note that a coroutine *could* ignore this signal, but it probably shouldn't. 
    * This signal is asserted against in the some parts of the library (this can be disabled, see below).
    * `BasicCoroutine`'s destructor will send this signal to the coroutine it encapsulates; failure to unwind the stack could lead to resource leaks since the coroutine won't be resumed again after the corresponding `BasicCoroutine` object is destroyed.
    * When calling `terminate()` or `.end()` on a coroutine object, that coroutine is resumed with a `YieldResult::Terminate` signal.

#### Free Function ` YieldResult yield_to(Coroutine& other)`
Suspend the currently-running coroutine and resume the `other` coroutine with a `Continue` signal.  It is safe to call this function when `other` is the currently-running coroutine.

#### Free Function `YieldResult yield_fast_to(Coroutine& other)` 
Suspend the currently-running coroutine and resume the `other` coroutine with a `Continue` signal.  It is **not** safe to call this function when `other` is the currently-running coroutine.  

#### Free Function `void terminate(Coroutine& other)` 
Suspend the currently-running coroutine and resume the `other` coroutine with a `Terminate` signal.  This is equivalent to calling `other.end()`. It is **not** safe to call this function when `other` is the currently-running coroutine.

#### Static Data Member `Coroutine Coroutine::main`
`Coroutine::main` is the `Coroutine` object corresponding to the `main` coroutine.  The main coroutine is special in that it does not need to have it's stack manually allocated and that it is always resumable in correct programs.

### Type `BasicCoroutine<class Callable, size_t StackSize>`
`BasicCoroutine` is a template type that encapsulates a coroutine-aware `Callable` object.  `BasicCoroutine` inherits from `Coroutine`.

`BasicCoroutine` is the type that users can use to instantiate new `Coroutine` objects.

The `Callable` parameter specifies the type of the callable object to call to start the coroutine.  An object type `Callable` is supplied on construction of the `BasicCoroutine` object which holds a copy of the object.  This object is called when `Coroutine::start()` is invoked.  The `Callable` object must be invocable with an lvalue reference to a `Coroutine` object and should have a `void` return type.

The `StackSize` parameter specifies the stack size, in bytes, to allocate for the Coroutine object.  Note that the coroutine's stack is a byte array living in the `BasicCoroutine` object itself; dynamic allocation of the coroutine stack is not supported.

Note that template parameters `Callable` and `StackSize` are deducible from all of `BasicCoroutine` constructors.

#### Constructor `BasicCoroutine<Callable, StackSize>::BasicCoroutine(Callable, stack_size<StackSize>)`
Construct a BasicCoroutine object from the callable object and given stack size.  This constructor has deduction guides to allow creating a new BasicCoroutine object without specifying the 'Callable' type and 'StackSize' explicitly:

```c++
void my_coroutine(Coroutine& coro);
auto coro = BasicCoroutine{my_coroutine, stack_size_v<128u>};
```

#### Constructor `BasicCoroutine<Callable, StackSize>::BasicCoroutine(Callable)`
Construct a BasicCoroutine object from the callable.  This constructor has deduction guides to allow creating a new BasicCoroutine object without specifying the 'Callable' type and 'StackSize' explicitly (the `StackSize` parameter defaults to 128):

```c++
void my_coroutine(Coroutine& coro);
auto coro = BasicCoroutine{my_coroutine};
```
#### Destructor `BasicCoroutine<Callable, StackSize>::~BasicCoroutine()`
Sends a terminate signal to the currently-running coroutine if it hasn't been terminated already.

### Tag Type `stack_size<size_t>`
An object of this tag type can be passed `BasicCoroutine`'s constructor to deduce its `StackSize` template parameter.

The template variable declaration is provided as a convenience:

```c++
template <size_t N>
inline constexpr auto stack_size_v = stack_size<N>{};
```

### Macro `TIM_CORO_NO_ASSERT`
Define this macro (or standard macro `NDEBUG`) before including `Coroutine.h` to disable assertions in `Coroutine.h`.  Note that the provided `libtimcoro.a` is compiled with assertions *enabled*.

# Requirements
Compiling this library requires a g++-compatible compiler that supports GCC's [extended asm statement](https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html) and C++17.
Including the `Coroutine.h` header requires a C++ compiler that supports C++17's [CTAD deduction guides](https://en.cppreference.com/w/cpp/language/class_template_argument_deduction) and [inline variable declarations](https://en.cppreference.com/w/cpp/language/inline).  

# License
This library is licensed under the MIT license.  See the LICENSE.txt file for details.

# Author 
Timothy VanSlyke - `vanslyke.t@husky.neu.edu`
