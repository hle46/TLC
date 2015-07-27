#ifndef IMTOOLBOX_MATRIX_H
#define IMTOOLBOX_MATRIX_H
#include <array>
#include <numeric>
#include <vector>
#include <algorithm>
#include <cassert>
#include <functional>

namespace imtoolbox {

//-----------------------------------------------------------------------------
//                            matrix concept
// What is the minimal set of conditions to check if a thing is a matrix. Here I
// propose to check if that thing has order (or dimension) and value type
//-----------------------------------------------------------------------------

// Safely deduce the result type of the expression x.order.
template <typename T> struct get_order_type {
private:
  template <typename X> static auto check(const X &x) -> decltype(x.order);

  static subst_failure check(...);

public:
  using type = decltype(check(std::declval<T>()));
};

// Safely deduce the result type of the expression x.order.
template <typename T> struct get_value_type {
private:
  template <typename X> static typename X::value_type check(const X &);

  static subst_failure check(...);

public:
  using type = decltype(check(std::declval<T>()));
};

// Returns true if the experssion x.order is valid.
template <typename T> constexpr bool has_order() {
  return subst_succeeded<typename get_order_type<T>::type>();
}

// Returns true if the experssion value_type is valid.
template <typename T> constexpr bool has_value_type() {
  return subst_succeeded<typename get_value_type<T>::type>();
}

// Put everything together to check if a thing is a matrix
template <typename M> constexpr bool is_matrix() {
  return has_order<M>() && has_value_type<M>();
}

// Put everything together to check if a thing is a 2D matrix
template <typename M> constexpr bool is_matrix2d() {
  return has_order<M>() && has_value_type<M>() && M::order == 2;
}

// Put everything together to check if a thing is a vector
template <typename V> constexpr bool is_vector() {
  return has_order<V>() && has_value_type<V>() && V::order == 1;
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//            matrix initialization using list data structure helpers
//-----------------------------------------------------------------------------

// matrix_init - recursive initializer_list used to initialize a matrix
// The matrix init trait defines the matrix initializer list for any matrix
// with order > 1
template <typename T, size_t N> struct matrix_init {
  using type = std::initializer_list<typename matrix_init<T, N - 1>::type>;
};

// specialization for order = 1
template <typename T> struct matrix_init<T, 1> {
  using type = std::initializer_list<T>;
};

// This is undefined on purpose!
template <typename T> struct matrix_init<T, 0>;

// matrix_initializer is an type alias used to initialize a matrix using a list
template <typename T, size_t N>
using matrix_initializer = typename matrix_init<T, N>::type;

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//              matrix initialization using list helper functions
//-----------------------------------------------------------------------------

template <size_t N> struct matrix_slice;

// check_non_jagged
// Used in derive_extents, returns true if the array is not jagged. That is,
// all sub-initializers of list must have the same size.
template <typename List> inline bool check_non_jagged(const List &list) {
  auto i = list.begin();
  for (auto j = i + 1, __end = list.end(); j != __end; ++j) {
    if (i->size() != j->size()) {
      return false;
    }
  }
  return true;
}

// add_extents
// derive_extents worker
// This matches when N == 1
template <size_t N, typename l, typename List>
inline enable_if_t<(N == 1), void> add_extents(l &first, const List &list) {
  *first++ = list.size();
}
// This matches when N > 1
template <size_t N, typename l, typename List>
inline enable_if_t<(N > 1), void> add_extents(l &first, const List &list) {
  assert(check_non_jagged(list));
  *first = list.size();
  add_extents<N - 1>(++first, *list.begin());
}

// derive_extents
// derive extents for matrix_slice from input arguments
template <size_t N, typename List>
inline std::array<size_t, N> derive_extents(const List &list) {
  std::array<size_t, N> ret;
  auto f = ret.begin();
  add_extents<N>(f, list); // put extents from list into f[]
  return ret;
}

// compute_strides
template <size_t N>
inline enable_if_t<(N >= 1), void> compute_strides(matrix_slice<N> &ms) {
  size_t st = 1;
  for (size_t i = N; i > 0; --i) {
    ms.strides[i - 1] = st;
    st *= ms.extents[i - 1];
  }
  ms.size = st;
}

// add_list
// insert_flat worker
template <typename T, typename Vec>
inline void add_list(const T *first, const T *last, Vec &vec) {
  vec.insert(vec.end(), first, last);
}
template <typename T, typename Vec>
inline void add_list(const std::initializer_list<T> *first,
                     const std::initializer_list<T> *last, Vec &vec) {
  for (; first != last; ++first) {
    add_list(first->begin(), first->end(), vec);
  }
}

// insert_flat
// Copy the elements from the initializer list nesting into contiguous
// elements in the vector.
template <typename T, typename Vec>
inline void insert_flat(const std::initializer_list<T> list, Vec &vec) {
  add_list(list.begin(), list.end(), vec);
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//                                 slice
// A slice describes a sequence of elements in some dimension (or row) of a
// matrix. It is a triple comprised of a starting index, a number of elements,
// and the stride between subsequent elements.
//
// The special members slice::all represent the selection of
// no or all elements in a particular dimension.
//------------------------------------------------------------------------------

struct slice {
  explicit slice(size_t s = 0, size_t e = std::numeric_limits<size_t>::max(),
                 size_t n = 1)
      : start{s}, end{e}, stride{n} {
    assert(start <= end);
  }

  static slice all;
  size_t start;
  size_t end;
  size_t stride;
};

//------------------------------------------------------------------------------

// requesting_element
// The subscript operator requests an element when all elements are
// convertible to size_t.
template <typename... Args> constexpr bool requesting_element() {
  return all(std::is_convertible<Args, size_t>::value...);
}

// requesting_slice
// The subscript operator requests an element when all elements are either
// convertible to size_t or slice or there are some slices
template <typename... Args> constexpr bool requesting_slice() {
  return all((std::is_convertible<Args, size_t>::value ||
              std::is_same<Args, slice>::value)...) &&
         some(std::is_same<Args, slice>::value...);
}

// check_bounds
// Returns true if each element in range is within the bounds of the
// corresponding extent.
template <size_t N, typename... Dims>
inline bool check_bounds(const matrix_slice<N> &ms, Dims... dims) {
  std::array<size_t, N> indexes{{static_cast<size_t>(dims)...}};
  return std::equal(indexes.begin(), indexes.end(), ms.extents.begin(),
                    std::less<size_t>());
}

// Compute an (N-1)D slice from an N-D slice. This is done by copying all
// extents and strides into the smaller-dimensional slice by excluding the
// Mth dimension. Note that:
//    If M == 0, this is called a row slice.
//    If M == 1, this is called a column slice.
//    If M == 2, this is a slice of the "z" plane.
template <size_t M, size_t N>
inline void slice_dim(size_t n, const matrix_slice<N> &in,
                      matrix_slice<N - 1> &out) {
  out.size = in.size / in.extents[M];       // scale the size of the matrix
  out.start = in.start + n * in.strides[M]; // compute the starting offset
  std::copy_n(in.extents.begin() + M + 1, N - M - 1,
              std::copy_n(in.extents.begin(), M, out.extents.begin()));
  std::copy_n(in.strides.begin() + M + 1, N - M - 1,
              std::copy_n(in.strides.begin(), M, out.strides.begin()));
}

template <size_t N>
inline void get_matrix_slice(size_t dim, size_t n, const matrix_slice<N> &in,
                             matrix_slice<N - 1> &out) {
  assert(dim < N);
  out.size = in.size / in.extents[dim];       // scale the size of the matrix
  out.start = in.start + n * in.strides[dim]; // compute the starting offset
  std::copy_n(in.extents.begin() + dim + 1, N - dim - 1,
              std::copy_n(in.extents.begin(), dim, out.extents.begin()));
  std::copy_n(in.strides.begin() + dim + 1, N - dim - 1,
              std::copy_n(in.strides.begin(), dim, out.strides.begin()));
}

// same extents
// The same_extents function returns true when two slices describe matrices
// with the same order and extents. The starting offset and strides do not
// factor into the comparison.
// An overload is provided for Matrix types. It compares the descriptors of its
// matrix arguments.
template <size_t M, size_t N>
inline bool same_extents(const matrix_slice<M> &a, const matrix_slice<N> &b) {
  return a.order == b.order &&
         std::equal(a.extents.begin(), a.extents.end(), b.extents.begin());
}

// same size
template <size_t M, size_t N>
inline bool same_size(const matrix_slice<M> &a, const matrix_slice<N> &b) {
  return a.size == b.size;
}

// Check if two matrixes have the same extent.
template <typename M1, typename M2>
inline enable_if_t<is_matrix<M1>() && is_matrix<M2>(), bool>
same_extents(const M1 &a, const M2 &b) {
  return same_extents(a.descriptor(), b.descriptor());
}

// Fast compute power using doubling algorithm
template <typename T> void fast_pow(T &x, int n) {
  if (n == 0) {
    x = 1;
    return;
  }
  if (n < 0) {
    x = 1 / x;
    n = -n;
  }
  T acc{1};
  while (n >= 1) {
    if (n % 2 == 1) {
      acc *= x;
    }
    x *= x;
    n >>= 1;
  }
  x = std::move(acc);
}

//-----------------------------------------------------------------------------
//                            matrix_slice
//-----------------------------------------------------------------------------
template <size_t N> struct matrix_slice {
  static constexpr size_t order = N;

  size_t start;                  // starting offset
  size_t size;                   // total number of elements
  std::array<size_t, N> extents; // number of elements in each dimension
  std::array<size_t, N> strides; // offset between elements in each dimension

  // Default constructor
  matrix_slice() : start{0}, size{0} {
    std::fill(extents.begin(), extents.end(), static_cast<size_t>(0));
    std::fill(strides.begin(), strides.end(), static_cast<size_t>(0));
  }

  // Move constructor
  matrix_slice(matrix_slice &&) = default;
  matrix_slice &operator=(matrix_slice &&) = default;

  // Copy semantics
  matrix_slice(const matrix_slice &) = default;
  matrix_slice &operator=(const matrix_slice &) = default;

  // Destructor
  ~matrix_slice() = default;

  matrix_slice(size_t s, std::initializer_list<size_t> exts) : start{s} {
    assert(exts.size() == N);
    std::copy(exts.begin(), exts.end(), extents.begin());
    compute_strides(*this);
  }

  matrix_slice(size_t s, const std::array<size_t, N> &exts) : start{s} {
    assert(exts.size() == N);
    std::copy(exts.begin(), exts.end(), extents.begin());
    compute_strides(*this);
  }

  template <typename... Args>
  enable_if_t<requesting_element<Args...>(), size_t>
  operator()(Args... args) const {
    static_assert(sizeof...(Args) == N, "");
    std::array<size_t, N> indexes{{static_cast<size_t>(args)...}};
    return start + std::inner_product(indexes.begin(), indexes.end(),
                                      strides.begin(), static_cast<size_t>(0));
  }

  // Calculate matrix_slice dimensions based on slice
  template <size_t D>
  size_t do_slice_dim(matrix_slice<N> &out, const slice &s) const {
    assert(s.start < extents[D]);
    size_t end;
    if (s.end == std::numeric_limits<size_t>::max()) {
      end = extents[D] - 1;
    } else {
      assert(s.end < extents[D]);
      end = s.end;
    }
    size_t length = ((end - s.start + 1) + s.stride - 1) / s.stride;
    out.extents[D] = length;
    out.strides[D] = strides[D] * s.stride;
    return s.start * strides[D];
  }

  // Calculate matrix_slice dimensions based on a number
  template <size_t D>
  size_t do_slice_dim(matrix_slice<N> &out, size_t n) const {
    assert(n < extents[D]);
    return do_slice_dim<D>(out, slice{n, n, 1});
  }

  template <typename... Args> size_t do_slice(matrix_slice<N> &) const {
    return 0;
  }

  template <typename T, typename... Args>
  size_t do_slice(matrix_slice<N> &out, const T &s,
                  const Args &... args) const {
    constexpr std::size_t D = N - sizeof...(Args) - 1;
    std::size_t m = do_slice_dim<D>(out, s);
    std::size_t n = do_slice(out, args...);
    return m + n; // accumulate to get the right start
  }

  template <typename... Args>
  enable_if_t<requesting_slice<Args...>(), matrix_slice<N>>
  operator()(const Args &... args) const {
    matrix_slice<N> d;
    d.start = start + do_slice(d, args...);
    d.size = std::accumulate(d.extents.begin(), d.extents.end(), 1,
                             std::multiplies<size_t>());
    return d;
  }

  /* Equality Comparison
   */
  friend bool operator==(const matrix_slice<N> &a, const matrix_slice<N> &b) {
    return a.start == b.start &&
           std::equal(a.extents.begin(), a.extents.end(), b.extents.begin()) &&
           std::equal(a.strides.begin(), a.strides.end(), b.strides.begin());
  }

  friend bool operator!=(const matrix_slice<N> &a, const matrix_slice<N> &b) {
    return !(a == b);
  }

  friend std::ostream &operator<<(std::ostream &os, const matrix_slice<N> &s) {
    os << '[' << s.start << ',' << s.size << ',' << '[';
    for (size_t i = 0; i < N - 1; ++i)
      os << s.extents[i] << ',';
    os << s.extents[N - 1] << ']' << ',' << '[';
    for (size_t i = 0; i < N - 1; ++i)
      os << s.strides[i] << ',';
    os << s.strides[N - 1] << ']' << ']';
    return os;
  }
};

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//                          slice_iterator
//-----------------------------------------------------------------------------

template <typename T, size_t N> class slice_iterator {
public:
  using value_type = typename std::remove_const<T>::type;
  using reference = T &;
  using pointer = T *;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;

  slice_iterator(const matrix_slice<N> &s, T *base, bool end_iter = false)
      : desc{std::cref(s)} {
    std::fill_n(indexes.begin(), N, 0);
    if (end_iter) {
      indexes[0] = s.extents[0];
      ptr = base + s.start + desc.get().extents[0] * desc.get().strides[0];
    } else {
      ptr = base + s.start;
    }
  }

  // Default constructor
  slice_iterator() = delete;

  // Copy
  slice_iterator(const slice_iterator &) = default;
  slice_iterator &operator=(const slice_iterator &other) = default;

  // Move
  slice_iterator(slice_iterator &&) = default;
  slice_iterator &operator=(slice_iterator &&other) = default;

  // Returns the iterators describing slice.
  const matrix_slice<N> &descriptor() const { return desc; }

  // Readable
  T &operator*() const { return *ptr; }
  T *operator->() const { return ptr; }

  // Forward Iterator
  slice_iterator &operator++() {
    increment();
    return *this;
  }

  slice_iterator operator++(int) {
    slice_iterator x = *this;
    increment();
    return x;
  }

  // Equality comparison
  // Two slice iterators are equal when they have the same descriptor and point
  // the the same first element
  friend bool operator==(const slice_iterator<T, N> &a,
                         const slice_iterator<T, N> &b) {
    assert(a.descriptor() == b.descriptor());
    return &*a == &*b;
  }

  friend bool operator!=(const slice_iterator<T, N> &a,
                         const slice_iterator<T, N> &b) {
    return !(a == b);
  }

private:
  std::reference_wrapper<const matrix_slice<N>>
      desc;                      // Describes the iterator range
  std::array<size_t, N> indexes; // Counting indexes
  T *ptr;                        // The current elements

  void increment() {
    const matrix_slice<N> &iter_desc = descriptor();
    std::size_t d = N - 1;
    while (true) {
      ptr += iter_desc.strides[d];
      ++indexes[d];

      // If have not yet counted to the extent of the current dimension, then
      // we will continue to do so in the next iteration.
      if (indexes[d] != iter_desc.extents[d])
        break;

      // Otherwise, if we have not counted to the extent in the outermost
      // dimension, move to the next dimension and try again. If d is 0, then
      // we have counted through the entire slice.
      if (d != 0) {
        ptr -= iter_desc.strides[d] * iter_desc.extents[d];
        indexes[d] = 0;
        --d;
      } else {
        break;
      }
    }
  }
};

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//                             matrix_ref
//-----------------------------------------------------------------------------

template <typename T, size_t N> class matrix;

template <typename T, size_t N> class matrix_ref {
public:
  static constexpr size_t order = N;
  using value_type = typename std::remove_const<T>::type;
  using iterator = slice_iterator<T, N>;
  using const_iterator = slice_iterator<const T, N>;

  // Default constructor
  matrix_ref() = delete;

  // Move constructor
  matrix_ref(matrix_ref &&) = default;
  matrix_ref &operator=(matrix_ref &&) = default;

  // Copy constructor
  matrix_ref(const matrix_ref &) = default;
  matrix_ref &operator=(const matrix_ref &) = default;

  // Destructor
  ~matrix_ref() = default;

  // Slice initialization
  matrix_ref(const matrix_slice<N> &s, T *p) : desc(s), ptr(p) {}

  matrix_ref(const matrix<value_type, N> &m)
      : desc(m.descriptor()), ptr(m.data()) {}

  matrix_ref &operator=(const matrix<value_type, N> &m) {
    assert(same_extents(desc, m.descriptor()));
    apply(m, [](T &a, const T &b) { a = b; });
    return *this;
  }

  // May lead to memory leak
  matrix_ref(matrix<value_type, N> &&) = delete;

  matrix<T, N> clone() const {
    matrix<T, N> ret(*this);
    return ret;
  }

  template <typename M> enable_if_t<is_matrix<M>(), void> copyTo(M &m) const {
    if (!same_extents(desc, m.descriptor())) {
      m = matrix<value_type, N>(desc);
    }
    std::copy(begin(), end(), m.begin());
  }

  // Return the slice describing the matrix
  const matrix_slice<N> &descriptor() const { return desc; }

  // Return nth dimension of the matrix
  size_t size(size_t n) const { return desc.extents[n]; }

  // Return total size
  size_t size() const { return desc.size; }

  // "flat" element access
  T *data() { return ptr; }
  const T *data() const { return ptr; }

  // Iterators
  iterator begin() { return {desc, ptr}; }
  iterator end() { return {desc, ptr, true}; }

  const_iterator begin() const { return {desc, ptr}; }
  const_iterator end() const { return {desc, ptr, true}; }

  // Subscripting
  template <typename... Args>
  enable_if_t<requesting_element<Args...>(), T &> operator()(Args... args) {
    static_assert(sizeof...(Args) == N, "Dimension mismatch");
    assert(check_bounds(desc, args...));
    return *(data() + desc(args...));
  }

  template <typename... Args>
  enable_if_t<requesting_element<Args...>(), const T &>
  operator()(Args... args) const {
    static_assert(sizeof...(Args) == N, "Dimension mismatch");
    assert(check_bounds(desc, args...));
    return *(data() + desc(args...));
  }

  // Slicing
  template <typename... Args>
  enable_if_t<requesting_slice<Args...>(), matrix_ref<T, N>>
  operator()(const Args &... args) {
    static_assert(sizeof...(Args) == N, "Dimension mismatch");
    return {desc(args...), data()};
  }

  template <typename... Args>
  enable_if_t<requesting_slice<Args...>(), matrix_ref<const T, N>>
  operator()(const Args &... args) const {
    static_assert(sizeof...(Args) == N, "Dimension mismatch");
    return {desc(args...), data()};
  }

  // [] row access
  matrix_ref<T, N - 1> operator[](size_t n) { return row(n); }
  matrix_ref<const T, N - 1> operator[](size_t n) const { return row(n); }

  // row access
  matrix_ref<T, N - 1> row(size_t n) {
    assert(n < size(0));
    matrix_slice<N - 1> row;
    slice_dim<0>(n, desc, row);
    return {row, data()};
  }

  matrix_ref<const T, N - 1> row(size_t n) const {
    assert(n < size(0));
    matrix_slice<N - 1> row;
    slice_dim<0>(n, desc, row);
    return {row, data()};
  }

  size_t rows() const { return size(0); }
  size_t cols() const { return size(1); }

  // Scalar application
  template <typename F> matrix_ref &apply(F f) {
    for (auto i = begin(), __end = end(); i != __end; ++i) {
      f(*i);
    }
    return *this;
  }

  // Apply 2 matrixes
  template <typename M, typename F>
  enable_if_t<is_matrix<M>(), matrix_ref &> apply(const M &m, F f) {
    assert(same_extents(desc, m.descriptor()));
    auto i = begin();
    auto j = m.begin();
    auto __end = end();
    while (i != __end) {
      f(*i, *j);
      ++i;
      ++j;
    }
    return *this;
  }

  // Scalar assignment
  matrix_ref &operator=(const T &x) {
    return apply([&](T &y) { y = x; });
  }

  // Scalar addition
  matrix_ref &operator+=(const T &x) {
    return apply([&](T &y) { y += x; });
  }

  // Scalar substraction
  matrix_ref &operator-=(const T &x) {
    return apply([&](T &y) { y -= x; });
  }

  // Scalar multiplication
  matrix_ref &operator*=(const T &x) {
    return apply([&](T &y) { y *= x; });
  }

  // Scalar division
  matrix_ref &operator/=(const T &x) {
    return apply([&](T &y) { y /= x; });
  }

  // Scalar power
  matrix_ref &operator^=(int x) {
    if (x == 0) {
      return apply([&](T &y) { y = 1; });
    }
    return apply([&](T &y) { fast_pow(y, x); });
  }

  // Matrix addition
  template <typename M>
  enable_if_t<is_matrix<M>(), matrix_ref &> operator+=(const M &m) {
    using U = typename M::value_type;
    return apply(m, [&](T &t, const U &u) { t += u; });
  }

  // Matrix subtraction
  template <typename M>
  enable_if_t<is_matrix<M>(), matrix_ref &> operator-=(const M &m) {
    using U = typename M::value_type;
    return apply(m, [&](T &t, const U &u) { t -= u; });
  }

  // Matrix multiplication
  template <typename M>
  enable_if_t<is_matrix<M>(), matrix_ref &> operator*=(const M &m) {
    using U = typename M::value_type;
    return apply(m, [&](T &t, const U &u) { t *= u; });
  }

  // Matrix division
  template <typename M>
  enable_if_t<is_matrix<M>(), matrix_ref &> operator/=(const M &m) {
    using U = typename M::value_type;
    return apply(m, [&](T &t, const U &u) { t /= u; });
  }

  // Swap
  void swap(matrix_ref &mr) {
    std::swap(desc, mr.desc);
    std::swap(ptr, mr.ptr);
  }

  // Swap rows
  void swap_rows(size_t m, size_t n) {
    auto r1 = (*this)[m];
    auto r2 = (*this)[n];
    std::swap_ranges(r1.begin(), r1.end(), r2.begin());
  }

private:
  matrix_slice<N> desc; // the shape of the matrix
  T *ptr;               // the first element in the matrix
};

// Zero-Dimension matrix_ref
// The type matrix_ref<T, 0> is not really a matrix. It contains a pointer
// to an element in a matrix.

template <typename T> class matrix_ref<T, 0> {
public:
  matrix_ref() = delete;

  matrix_ref(const matrix_slice<0> &s, T *p) : ptr(p + s.start) {}

  // Modifying operators
  matrix_ref &operator=(const T &x) {
    *ptr = x;
    return *this;
  }

  T &operator()() { return *ptr; }
  T &operator()() const { return *ptr; }

  operator T &() { return *ptr; }
  operator T &() const { return *ptr; }

private:
  T *ptr;
};

//------------------------------------------------------------------------------
//                                matrix
// Template parameters:
//     T -- Type of matrix elements.
//     N -- Dimension of the matrix.
//
// Sample contructor usages:
//    + Construct from a list:
//          matrix<int, 2> m({{1, 2, 3}, {4, 5, 6}, {7, 8, 9}});
//
//    + Construct from a matrix_ref which is a result of a slicing
//          matrix<int, 2> m1(m(slice{1, 2}, slice{1, 2}));
//
//    + Construct by specifying dimensions:
//          matrix<int, 2> m2(3, 4);
//
// Sample assignment usages:
//    + Assign from a list
//          matrix<int, 2> m = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
//
//    + Assign from a matrix_ref which is a result of a slicing
//          matrix<int, 2> m1 = m(slice{1, 2}, slice{1, 2});
//-----------------------------------------------------------------------------

template <typename T, size_t N> class matrix {
public:
  static constexpr size_t order = N;
  using value_type = T;
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;

  // Default constructor
  matrix() = default;

  // Destructor
  ~matrix() = default;

  // Move semantics
  matrix(matrix &&) = default;
  matrix &operator=(matrix &&) = default;

  // Copy semantics
  matrix(const matrix &) = delete;
  matrix &operator=(const matrix &) = delete;

  // Construct by initializing from list
  matrix(matrix_initializer<T, N> init) {
    desc.extents = derive_extents<N>(init);
    compute_strides(desc);
    elems.reserve(desc.size);
    insert_flat(init, elems);
    assert(elems.size() == desc.size);
  }

  // Assign from list
  template <typename U> matrix &operator=(matrix_initializer<T, N> init) {
    matrix tmp{init};
    swap(tmp);
    return *this;
  }

  // Construct from matrix_ref
  template <typename U>
  matrix(const matrix_ref<U, N> &mr)
      : desc(mr.descriptor()), elems(desc.size) {
    static_assert(is_same<common_type_t<U, T>, T>(),
                  "Incompatible element types");
    // Since matrix_ref is just a reference to an actual matrix and has no
    // storage, we need to recompute strides here
    compute_strides(desc);
    // Copy elements
    std::copy(mr.begin(), mr.end(), elems.begin());
  }

  // Assign from matrix_ref
  template <typename U> matrix &operator=(const matrix_ref<U, N> &mr) {
    static_assert(std::is_convertible<U, T>::value,
                  "Incompatible element types");
    desc = mr.descriptor();
    elems.resize(desc.size);
    // Since matrix_ref is just a reference to an actual matrix and has no
    // storage, we need to recompute strides here
    compute_strides(desc);
    // Copy elements
    std::copy(mr.begin(), mr.end(), elems.begin());
    return *this;
  }

  // Slice initialization
  // Initialize the matrix so that it has the same extents as the given
  // slice. Note that the strides are not copied. The resulting matrix
  // indexes it's elements in row-major order.
  explicit matrix(const matrix_slice<N> &ms)
      : desc(0, ms.extents), elems(desc.size) {
    // Compact representation just in case the matrix_slice is from matrix_ref
    compute_strides(desc);
  }
  // Init with a value version
  explicit matrix(const matrix_slice<N> &ms, const T &init)
      : desc(0, ms.extents), elems(desc.size, init) {
    // Compact representation just in case the matrix_slice is from matrix_ref
    compute_strides(desc);
  }

  matrix clone() const {
    matrix ret;
    ret.desc = desc;
    ret.elems = elems;
    return ret;
  }

  template <typename M> enable_if_t<is_matrix<M>(), void> copyTo(M &m) const {
    if (!same_extents(desc, m.descriptor())) {
      m = matrix<value_type, N>(desc);
    }
    std::copy(begin(), end(), m.begin());
  }

  // Construct by specifying dimensions
  template <typename... Dims>
  explicit matrix(Dims... dims)
      : desc(0, {static_cast<size_t>(dims)...}), elems(desc.size, 0) {}

  template <typename U> matrix(std::initializer_list<U>) = delete;
  template <typename U> matrix &operator=(std::initializer_list<U>) = delete;

  // Return the slice describing the matrix
  const matrix_slice<N> &descriptor() const { return desc; }

  // Return nth dimension of the matrix
  size_t size(size_t n) const { return desc.extents[n]; }

  // Return total size
  size_t size() const { return elems.size(); }

  // "flat" element access
  T *data() { return elems.data(); }
  const T *data() const { return elems.data(); }

  // Iterators
  iterator begin() { return elems.begin(); }
  iterator end() { return elems.end(); }

  const_iterator begin() const { return elems.begin(); }
  const_iterator end() const { return elems.end(); }

  // Subscripting
  template <typename... Args>
  enable_if_t<requesting_element<Args...>(), T &> operator()(Args... args) {
    static_assert(sizeof...(Args) == N, "Dimension mismatch");
    assert(check_bounds(desc, args...));
    return *(data() + desc(args...));
  }

  template <typename... Args>
  enable_if_t<requesting_element<Args...>(), const T &>
  operator()(Args... args) const {
    static_assert(sizeof...(Args) == N, "Dimension mismatch");
    assert(check_bounds(desc, args...));
    return *(data() + desc(args...));
  }

  // Slicing
  template <typename... Args>
  enable_if_t<requesting_slice<Args...>(), matrix_ref<T, N>>
  operator()(const Args &... args) {
    static_assert(sizeof...(Args) == N, "Dimension mismatch");
    return {desc(args...), data()};
  }

  template <typename... Args>
  enable_if_t<requesting_slice<Args...>(), matrix_ref<const T, N>>
  operator()(const Args &... args) const {
    static_assert(sizeof...(Args) == N, "Dimension mismatch");
    return {desc(args...), data()};
  }

  // [] row access
  matrix_ref<T, N - 1> operator[](size_t n) { return row(n); }
  matrix_ref<const T, N - 1> operator[](size_t n) const { return row(n); }

  // row access
  matrix_ref<T, N - 1> row(size_t n) {
    assert(n < size(0));
    matrix_slice<N - 1> ms;
    slice_dim<0>(n, desc, ms);
    return {ms, data()};
  }

  matrix_ref<const T, N - 1> row(size_t n) const {
    assert(n < size(0));
    matrix_slice<N - 1> ms;
    slice_dim<0>(n, desc, ms);
    return {ms, data()};
  }

  size_t rows() const { return size(0); }
  size_t cols() const { return size(1); }

  // Scalar application
  template <typename F> matrix &apply(F f) {
    for (auto &x : elems) {
      f(x);
    }
    return *this;
  }

  // Apply 2 matrixes
  template <typename M, typename F>
  enable_if_t<is_matrix<M>(), matrix &> apply(const M &m, F f) {
    assert(same_extents(desc, m.descriptor()));
    auto i = begin();
    auto j = m.begin();
    auto __end = end();
    while (i != __end) {
      f(*i, *j);
      ++i;
      ++j;
    }
    return *this;
  }

  // Scalar assignment
  matrix &operator=(const T &x) {
    return apply([&](T &y) { y = x; });
  }

  // Scalar addition
  matrix &operator+=(const T &x) {
    return apply([&](T &y) { y += x; });
  }

  // Scalar substraction
  matrix &operator-=(const T &x) {
    return apply([&](T &y) { y -= x; });
  }

  // Scalar multiplication
  matrix &operator*=(const T &x) {
    return apply([&](T &y) { y *= x; });
  }

  // Scalar division
  matrix &operator/=(const T &x) {
    return apply([&](T &y) { y /= x; });
  }

  // Scalar power
  matrix &operator^=(int x) {
    if (x == 0) {
      return apply([&](T &y) { y = 1; });
    }
    return apply([&](T &y) { fast_pow(y, x); });
  }

  // Matrix addition
  template <typename M>
  enable_if_t<is_matrix<M>(), matrix &> operator+=(const M &m) {
    using U = typename M::value_type;
    return apply(m, [&](T &t, const U &u) { t += u; });
  }

  // Matrix subtraction
  template <typename M>
  enable_if_t<is_matrix<M>(), matrix &> operator-=(const M &m) {
    using U = typename M::value_type;
    return apply(m, [&](T &t, const U &u) { t -= u; });
  }

  // Matrix multiplication
  template <typename M>
  enable_if_t<is_matrix<M>(), matrix &> operator*=(const M &m) {
    using U = typename M::value_type;
    return apply(m, [&](T &t, const U &u) { t *= u; });
  }

  // Matrix division
  template <typename M>
  enable_if_t<is_matrix<M>(), matrix &> operator/=(const M &m) {
    using U = typename M::value_type;
    return apply(m, [&](T &t, const U &u) { t /= u; });
  }

  void swap(matrix &m) {
    std::swap(desc, m.desc);
    elems.swap(m.elems);
  }

  void swap_rows(size_t m, size_t n) {
    auto r1 = (*this)[m];
    auto r2 = (*this)[n];
    std::swap_ranges(r1.begin(), r1.end(), r2.begin());
  }

  template <size_t M, typename U, size_t P>
          friend enable_if_t < M >= 1 &&
      M<P, matrix<U, M>> resize(matrix<U, P> &in);

  template <size_t M, typename U, size_t P>
  friend enable_if_t<M >= 1 && M == P, matrix<U, M>> resize(matrix<U, P> &in);

private:
  matrix_slice<N> desc; // slice defining extents in the N dimensions
  std::vector<T> elems;
};

// Zero-Dimension Matrix
// The type matrix<T, 0> is not really a matrix. It stores a single scalar
// of type T and can only be converted to a reference to that type.

template <typename T> class matrix<T, 0> {
public:
  matrix() = default;

  // Initialize the pseudo-matrix.
  matrix(const T &x) : elem(x) {}

  matrix &operator=(const T &value) {
    elem = value;
    return *this;
  }

  T &operator()() { return elem; }
  const T &operator()() const { return elem; }

  operator T &() { return elem; }
  operator const T &() const { return elem; }

private:
  T elem;
};

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//                            matrix operations
//-----------------------------------------------------------------------------

// Check for zero dimension matrix
template <typename M> enable_if_t<is_matrix<M>(), bool> is_empty(const M &m) {
  return std::any_of(m.descriptor().extents.begin(),
                     m.descriptor().extents.end(),
                     [](size_t x) { return x == 0; });
}

template <typename T, size_t N> matrix<T, N> empty_matrix() {
  return matrix<T, N>(matrix_slice<N>{});
}

// Equality Comparison
// Two matrices compare equal when they have the same elements. Comparing
// matrices described by different slices is undefined behavior.
template <typename M1, typename M2>
inline enable_if_t<
    is_matrix<M1>() && is_matrix<M2>() &&
        is_same<typename M1::value_type, typename M2::value_type>() &&
        is_integral<typename M1::value_type>(),
    bool>
operator==(const M1 &a, const M2 &b) {
  if (is_empty(a) && is_empty(b)) {
    return true;
  }
  assert(same_extents(a, b));
  return std::equal(a.begin(), a.end(), b.begin());
}

template <typename M1, typename M2>
inline enable_if_t<
    is_matrix<M1>() && is_matrix<M2>() &&
        is_same<typename M1::value_type, typename M2::value_type>() &&
        is_floating_point<typename M1::value_type>(),
    bool>
operator==(const M1 &a, const M2 &b) {
  if (is_empty(a) && is_empty(b)) {
    return true;
  }
  assert(same_extents(a, b));
  return std::equal(a.begin(), a.end(), b.begin(),
                    [](auto x, auto y) { return almost_equal(x, y); });
}

template <typename M1, typename M2>
inline enable_if_t<is_matrix<M1>() && is_matrix<M2>(), bool>
operator!=(const M1 &a, const M2 &b) {
  return !(a == b);
}

template <typename M1, typename M2>
inline enable_if_t<
    is_matrix<M1>() && is_matrix<M2>() && M1::order == M2::order,
    matrix<common_type_t<typename M1::value_type, typename M2::value_type>,
           M1::order>>
operator/(const M1 &m1, const M2 &m2) {
  assert(same_extents(m1, m2));
  using T = common_type_t<typename M1::value_type, typename M2::value_type>;
  constexpr size_t N = M1::order;
  matrix<T, N> ret(m1.descriptor());
  m1.copyTo(ret);
  ret /= m2;
  return ret;
}

// overload constant minus a matrix
template <typename M, typename T>
inline enable_if_t<is_matrix<M>(),
                   matrix<common_type_t<T, typename M::value_type>, M::order>>
operator-(const T &x, const M &m) {
  using value_t = common_type_t<T, typename M::value_type>;
  constexpr size_t N = M::order;
  matrix<value_t, N> ret(m.descriptor());
  m.copyTo(ret);
  ret.apply([&](value_t &y) { y = x - y; });
  return ret;
}

// overload exponential some matrix
template <typename M>
inline enable_if_t<is_matrix<M>(), matrix<typename M::value_type, M::order>>
operator^(const M &m, int n) {
  auto ret = m.clone();
  ret ^= n;
  return ret;
}

// Scalar threshold
template <typename M, typename T>
inline enable_if_t<is_matrix<M>(), matrix<logical, M::order>>
operator<=(const M &m, const T &x) {
  // using value_t = common_type_t<typename M::value_type, T>;
  matrix<logical, M::order> ret(m.descriptor());
  auto p = ret.begin();
  for (auto first = m.begin(), last = m.end(); first != last; ++first) {
    *p = (*first <= x) ? 1 : 0;
    ++p;
  }
  return ret;
}

// Scalar threshold
template <typename M, typename T>
inline enable_if_t<is_matrix<M>(), matrix<logical, M::order>>
operator<(const M &m, const T &x) {
  // using value_t = common_type_t<typename M::value_type, T>;
  matrix<logical, M::order> ret(m.descriptor());
  auto p = ret.begin();
  for (auto first = m.begin(), last = m.end(); first != last; ++first) {
    *p = (*first < x) ? 1 : 0;
    ++p;
  }
  return ret;
}

// Scalar threshold
template <typename M, typename T>
inline enable_if_t<is_matrix<M>(), matrix<logical, M::order>>
operator>=(const M &m, const T &x) {
  // using value_t = common_type_t<typename M::value_type, T>;
  matrix<logical, M::order> ret(m.descriptor());
  auto p = ret.begin();
  for (auto first = m.begin(), last = m.end(); first != last; ++first) {
    *p = (*first >= x) ? 1 : 0;
    ++p;
  }
  return ret;
}

// Scalar threshold
template <typename M, typename T>
inline enable_if_t<is_matrix<M>(), matrix<logical, M::order>>
operator>(const M &m, const T &x) {
  // using value_t = common_type_t<typename M::value_type, T>;
  matrix<logical, M::order> ret(m.descriptor());
  auto p = ret.begin();
  for (auto first = m.begin(), last = m.end(); first != last; ++first) {
    *p = (*first > x) ? 1 : 0;
    ++p;
  }
  return ret;
}

// Printing function for debug
#ifdef DEBUG
template <typename M>
enable_if_t<is_matrix<M>(), std::ostream &> operator<<(std::ostream &os,
                                                       const M &m) {
  os << '{';
  for (size_t i = 0; i < m.size(0); ++i) {
    os << m[i];
    if ((i + 1) < m.size(0)) {
      os << ',';
    }
  }
  return (os << '}');
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const matrix_ref<T, 0> &mr) {
  os << static_cast<T>(mr);
  return os;
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const matrix<T, 0> &m) {
  os << static_cast<T>(m);
  return os;
}
#endif

template <size_t M, typename T, size_t N>
        inline enable_if_t < M >= 1 &&
    M<N, matrix<T, M>> resize(matrix<T, N> &in) {
  matrix_slice<M> out_ms;
  out_ms.start = in.desc.start;
  auto n = M - 1;
  auto first = in.desc.extents.begin();
  auto result = out_ms.extents.begin();
  while (n != 0) {
    *result = *first;
    ++result;
    ++first;
    --n;
  }
  *result = std::accumulate(first, in.desc.extents.end(), 1,
                            std::multiplies<size_t>());
  compute_strides(out_ms);
  matrix<T, M> out(out_ms);
  out.elems = std::move(in.elems);
  return out;
}

template <size_t M, typename T, size_t N>
inline enable_if_t<M >= 1 && M == N, matrix<T, M>> resize(matrix<T, N> &in) {
  return std::move(in);
}

//-----------------------------------------------------------------------------

template <typename T> using vector = matrix<T, 1>;
template <typename T> using matrix2 = matrix<T, 2>;
template <typename T> using matrix3 = matrix<T, 3>;

} // namespace imtoolbox;
#endif // IMTOOLBOX_MATRIX_H
