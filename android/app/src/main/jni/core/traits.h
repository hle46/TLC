#ifndef IMTOOLBOX_TRAITS_H
#define IMTOOLBOX_TRAITS_H

#include <type_traits>
#include <limits>
#include <cmath>

namespace imtoolbox {

template <bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template <bool B, class T, class F>
using conditional_t = typename std::conditional<B, T, F>::type;

template <typename T> constexpr bool is_arithmetic() {
  return std::is_arithmetic<T>::value;
}

template <typename T> constexpr bool is_integral() {
  return std::is_integral<T>::value;
}

template <typename T> constexpr bool is_floating_point() {
  return std::is_floating_point<T>::value;
}

template <typename T, typename U> constexpr bool is_same() {
  return std::is_same<T, U>::value;
}

template <typename T, typename U> constexpr bool is_convertible() {
  return std::is_convertible<T, U>::value;
}

template <typename... T>
using common_type_t = typename std::common_type<T...>::type;

struct subst_failure {};

// Returns true if T does not indicate a substitution failure.
template <typename T> constexpr bool subst_succeeded() {
  return !std::is_same<T, subst_failure>::value;
}

// bool_constant
// A helper alias template of std::integral_constant defined for bool type
template <bool B> using bool_constant = std::integral_constant<bool, B>;

// all
// Return true if every argument is true or if no argument is given
constexpr bool all() { return true; }

template <typename... Args> constexpr bool all(bool b, Args... args) {
  return b && all(args...);
}

// some
// Return true if some argument (at least one) is true
constexpr bool some() { return false; }

template <typename... Args> constexpr bool some(bool b, Args... args) {
  return b || some(args...);
}

// are_same
// Return true if all argument types are the same, or no argument is given
// This is an extended version of std::is_same
template <typename... Args> struct are_same;

template <> struct are_same<> : std::true_type {};

template <typename T> struct are_same<T> : std::true_type {};

template <typename T, typename U, typename... Args>
struct are_same<T, U, Args...>
    : bool_constant<std::is_same<T, U>::value && are_same<U, Args...>::value> {
};
} // namespace imtoolbox
#endif // IMTOOLBOX_TRAITS_H
