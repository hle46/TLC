#ifndef IMTOOLBOX_ULTILITY_H
#define IMTOOLBOX_ULTILITY_H

#include <iostream>
#include <fstream>
#include <cassert>

#ifdef ANDROID
#include <android/log.h>
#define LOGI(...)                                                              \
  __android_log_print(ANDROID_LOG_INFO, "BIOASSAY NATIVE", __VA_ARGS__)
#define LOGD(...)                                                              \
  __android_log_print(ANDROID_LOG_DEBUG, "BIOASSAY NATIVE", __VA_ARGS__)
#define LOGE(...)                                                              \
  __android_log_print(ANDROID_LOG_ERROR, "BIOASSAY NATIVE", __VA_ARGS__)
#endif

namespace imtoolbox {

#ifdef DEBUG
namespace os_color {
constexpr const char *def = "\033[0m";
constexpr const char *red = "\033[1;31m";
constexpr const char *green = "\033[1;32m";
constexpr const char *yellow = "\033[1;33m";
constexpr const char *blue = "\033[1;34m";
constexpr const char *magenta = "\033[1;35m";
constexpr const char *cyan = "\033[1;36m";
constexpr const char *white = "\033[1;37m";
} // namespace os_color

inline void print_helper() {}

template <typename T> inline void print_helper(std::ostream &os, const T &t) {
  os << t;
}

template <typename T, typename... Args>
inline void print_helper(std::ostream &os, const T &t, const Args &... args) {
  os << t << ", ";
  print_helper(os, args...);
}

template <typename... Args> inline void print_e(const Args &... args) {
  std::cerr << os_color::red;
  print_helper(std::cerr, args...);
  std::cerr << os_color::def;
}

template <typename... Args> inline void println_e(const Args &... args) {
  print_e(args...);
  std::cerr << "\n";
}

template <typename... Args> inline void print_i(const Args &... args) {
  std::cout << os_color::magenta;
  print_helper(std::cout, args...);
  std::cout << os_color::def;
}

template <typename... Args> inline void println_i(const Args &... args) {
  print_i(args...);
  std::cout << "\n";
}

template <typename... Args> inline void print_w(const Args &... args) {
  std::cout << os_color::yellow;
  print_helper(std::cout, args...);
  std::cout << os_color::def;
}

template <typename... Args> inline void println_w(const Args &... args) {
  print_w(args...);
  std::cout << "\n";
}

#else

// Write to log file in release mode
extern std::ofstream log;

inline void begin_log(const char *file_name) {
  if (log.is_open()) {
    log.close();
  }

  log.open(file_name, std::fstream::out | std::fstream::app);
}

inline void begin_log(const std::string &file_name) {
  begin_log(file_name.c_str());
}

inline void end_log() {
  if (log.is_open()) {
    log.close();
  }
}

inline void print_helper() {}

template <typename T> inline void print_helper(std::ofstream &ofs, const T &t) {
  ofs << t;
}

template <typename T, typename... Args>
inline void print_helper(std::ofstream &ofs, const T &t, const Args &... args) {
  ofs << t << ", ";
  print_helper(ofs, args...);
}

template <typename... Args> inline void print_e(const Args &... args) {
  log << "**ERROR**: ";
  print_helper(log, args...);
}

template <typename... Args> inline void println_e(const Args &... args) {
  print_e(args...);
  log << "\n";
}

template <typename... Args> inline void print_i(const Args &... args) {
  log << "**INFO**: ";
  print_helper(log, args...);
}

template <typename... Args> inline void println_i(const Args &... args) {
  print_i(args...);
  log << "\n";
}

template <typename... Args> inline void print_w(const Args &... args) {
  log << "**WARN**: ";
  print_helper(log, args...);
}

template <typename... Args> inline void println_w(const Args &... args) {
  print_w(args...);
  log << "\n";
}

#endif // DEBUG

// This is used to ignore compiler warning of unused variables
template <typename T> void ignore(const T &) {}

// almost_equal
// Floating point comparison
template <typename T>
inline typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
almost_equal(const T &x, const T &y, int ulp = 1) {
  // the machine epsilon has to be scaled to the magnitude of the values used
  // and multiplied by the desired precision in ULPs (units in the last place)
  return std::abs(x - y) <
             std::numeric_limits<T>::epsilon() * std::abs(x + y) * ulp
         // unless the result is subnormal
         ||
         std::abs(x - y) < std::numeric_limits<T>::min();
}

// minmax_element
// Finds the greatest and the smallest element in the range [first, last),
// ignoring elements which do not satisfy the predicate f
template <typename ForwardIterator, typename F>
std::pair<ForwardIterator, ForwardIterator>
minmax_element(ForwardIterator first, ForwardIterator last, F f) {
  while (first != last && !f(*first)) {
    ++first;
  }

  if (first == last) {
    return std::make_pair(first, first);
  }

  auto largest = first;
  auto smallest = first;
  while (++first != last) {
    if (f(*first)) {
      if (*first > *largest) {
        largest = first;
      }
      if (*first < *smallest) {
        smallest = first;
      }
    }
  }
  return std::make_pair(smallest, largest);
}

template <typename ForwardIterator>
std::pair<ForwardIterator, ForwardIterator>
minmax_element(ForwardIterator first, ForwardIterator last) {
  if (first == last) {
    return std::make_pair(first, first);
  }

  auto largest = first;
  auto smallest = first;
  while (++first != last) {
    if (*first > *largest) {
      largest = first;
    }
    if (*first < *smallest) {
      smallest = first;
    }
  }
  return std::make_pair(smallest, largest);
}

// max_element
// Finds the greatest element in the range [first, last)
template <typename ForwardIterator>
std::pair<ForwardIterator, size_t> max_element(ForwardIterator first,
                                               ForwardIterator last) {
  if (first == last) {
    return std::make_pair(first, -1);
  }
  size_t idx = 0;
  auto max_idx = idx;
  ForwardIterator next = first;
  while (++next != last) {
    ++idx;
    if (*first < *next) {
      first = next;
      max_idx = idx;
    }
  }

  return std::make_pair(first, max_idx);
}

// max_element
// Finds the greatest element in the range [first, last), ignoring elements
// which do not satisfy the predicate f
template <typename ForwardIterator, typename F>
std::pair<ForwardIterator, size_t> max_element(ForwardIterator first,
                                               ForwardIterator last, F f) {
  size_t idx = 0;
  while (first != last && !f(*first)) {
    ++first;
    ++idx;
  }
  if (first == last) {
    return std::make_pair(first, -1);
  }

  auto max_idx = idx;
  ForwardIterator next = first;
  while (++next != last) {
    ++idx;
    if (f(*next) && *first < *next) {
      first = next;
      max_idx = idx;
    }
  }

  return std::make_pair(first, max_idx);
}

enum class sort_t { none, descend, ascend };

using logical = uint8_t;

using fp_t = double;

struct Rect {
  size_t x1;
  size_t x2;
  size_t y1;
  size_t y2;
  Rect(size_t _x1, size_t _x2, size_t _y1, size_t _y2)
      : x1(_x1), x2(_x2), y1(_y1), y2(_y2) {}
};

struct Margin {
  size_t left;
  size_t right;
  size_t top;
  size_t bottom;
  Margin(size_t l, size_t r, size_t t, size_t b)
      : left(l), right(r), top(t), bottom(b) {}
};

constexpr int get_exponent(size_t n) {
  int exponent = 0;
  while ((n >>= 1) != 0) {
    ++exponent;
  }
  return exponent;
}

// Function signatures
bool exist(const char *file_name) noexcept;
bool exist(const std::string &file_name) noexcept;
} // namespace imtoolbox
#endif // IMTOOLBOX_ULTILITY_H
