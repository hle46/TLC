#ifndef IMTOOLBOX_PROC
#define IMTOOLBOX_PROC

#include <queue>
#ifdef ANDROID
#include <arm_neon.h>
#endif
namespace imtoolbox {

template <size_t num_images, size_t N = 3>
matrix<uint8_t, N> avg_folder(const char *path_name, const char *avg_file_name,
                              const Margin &margin) noexcept {
  static_assert(N == 2 || N == 3, "N == 2 || N == 3");
  static_assert(num_images >= 1 && num_images <= 256,
                "num_images >= 1 && num_images <= 256");
  static_assert((num_images & (num_images - 1)) == 0,
                "number of images should be a power of 2");
  std::string path{path_name};
  if (path[path.length() - 1] != '/') {
    path += "/";
  }

  std::vector<matrix<uint8_t, N>> images(num_images);
  for (size_t i = 0; i < num_images; ++i) {
    images[i] = imread<uint8_t, N>(path + std::to_string(i + 1) + ".jpg");
  }
  matrix<uint8_t, N> empty_mat{};
  if (images[0].size() == 0) {
    return empty_mat;
  }
  for (size_t i = 1; i < num_images; ++i) {
    for (size_t j = 0; j < N; ++j) {
      if (images[i].size(j) != images[0].size(j)) {
        return empty_mat;
      }
    }
  }

  size_t height = images[0].size(0);
  size_t width = images[0].size(1);
  size_t depth = (N == 3) ? images[0].size(2) : 1;

  if (margin.top >= height || margin.bottom >= height || margin.left >= width ||
      margin.right >= width) {
    return empty_mat;
  }
  size_t row_start = margin.top;
  size_t row_stop = height - 1 - margin.bottom;
  size_t col_start = margin.left;
  size_t col_stop = width - 1 - margin.right;
  if (row_start > row_stop || col_start > col_stop) {
    return empty_mat;
  }

  size_t row_stride = width * depth;

  size_t offset_from_start = row_start * row_stride + col_start * depth;
  std::vector<uint8_t *> pix(num_images);
  for (size_t i = 0; i < num_images; ++i) {
    pix[i] = images[i].data() + offset_from_start;
  }

  size_t next_row_offset = (margin.left + margin.right) * depth;
  auto ret = matrix<uint8_t, 3>(row_stop - row_start + 1,
                                col_stop - col_start + 1, depth);
  uint8_t *pix_ret = ret.data();
  size_t n = (col_stop - col_start + 1) * depth;
#ifdef ANDROID
  constexpr int exponent = get_exponent(num_images);
  uint16x8_t temp;
  uint8x8_t pad = vdup_n_u8(num_images / 2);
  size_t remain = n % 8;
  for (size_t k = row_start; k <= row_stop; ++k) {
    for (size_t i = 0; i < n / 8; ++i) {
      temp = vdupq_n_u16(0);
      uint8x8_t pix_val;
      for (size_t j = 0; j < num_images; ++j) {
        pix_val = vld1_u8(pix[j]);
        temp = vaddw_u8(temp, pix_val);
        pix[j] += 8;
      }
      temp = vaddw_u8(temp, pad);
      vst1_u8(pix_ret, vqmovn_u16(vshrq_n_u16(temp, exponent)));
      pix_ret += 8;
    }
    if (remain > 0) {
      // Overlap
      for (size_t j = 0; j < num_images; ++j) {
        pix[j] -= (8 - remain);
      }
      pix_ret -= (8 - remain);
      temp = vdupq_n_u16(0);
      uint8x8_t pix_val;
      for (size_t j = 0; j < num_images; ++j) {
        pix_val = vld1_u8(pix[j]);
        temp = vaddw_u8(temp, pix_val);
        pix[j] += 8;
      }
      temp = vaddw_u8(temp, pad);
      vst1_u8(pix_ret, vqmovn_u16(vshrq_n_u16(temp, exponent)));
      pix_ret += 8;
    }
    for (size_t j = 0; j < num_images; ++j) {
      pix[j] += next_row_offset;
    }
  }
#else
  uint16_t s = 0;
  uint8_t pad = num_images / 2;
  for (size_t k = row_start; k <= row_stop; ++k) {
    for (size_t i = 0; i < n; ++i) {
      s = 0;
      for (size_t j = 0; j < num_images; ++j) {
        s += *(pix[j]++);
      }
      *pix_ret++ = (s + pad) / num_images;
    }
    for (size_t j = 0; j < num_images; ++j) {
      pix[j] += next_row_offset;
    }
  }
#endif // ANDROID
  ret = resize<N>(ret);
  imwrite(ret, path + avg_file_name);
  return ret;
}

// Template functions
template <size_t num_images, size_t N = 3>
matrix<uint8_t, N> avg_folder(const char *path_name,
                              const char *avg_file_name) noexcept {
  static_assert(N == 2 || N == 3, "N == 2 || N == 3");
  static_assert(num_images >= 1 && num_images <= 256,
                "num_images >= 1 && num_images <= 256");
  static_assert((num_images & (num_images - 1)) == 0,
                "number of images should be a power of 2");
  std::string path{path_name};
  if (path[path.length() - 1] != '/') {
    path += "/";
  }
  std::vector<matrix<uint8_t, N>> images(num_images);
  for (size_t i = 0; i < images.size(); ++i) {
    images[i] = imread<uint8_t, N>(path + std::to_string(i + 1) + ".jpg");
  }

  matrix<uint8_t, N> empty_mat{};
  if (images[0].size() == 0) {
    return empty_mat;
  }
  for (size_t i = 1; i < num_images; ++i) {
    for (size_t j = 0; j < N; ++j) {
      if (images[i].size(j) != images[0].size(j)) {
        return empty_mat;
      }
    }
  }

  std::vector<uint8_t *> pix(num_images);
  for (size_t i = 0; i < num_images; ++i) {
    pix[i] = images[i].data();
  }

#ifdef ANDROID
  size_t n = images[0].size();
  constexpr int exponent = get_exponent(num_images);
  uint16x8_t temp;
  uint8x8_t pad = vdup_n_u8(num_images / 2);
  for (size_t i = 0; i < n / 8; ++i) {
    temp = vdupq_n_u16(0);
    uint8x8_t pix_val;
    for (size_t j = 0; j < num_images; ++j) {
      pix_val = vld1_u8(pix[j]);
      temp = vaddw_u8(temp, pix_val);
    }
    temp = vaddw_u8(temp, pad);
    vst1_u8(pix[num_images - 1], vqmovn_u16(vshrq_n_u16(temp, exponent)));
    for (size_t j = 0; j < num_images; ++j) {
      pix[j] += 8;
    }
  }
  size_t remain = n % 8;
  if (remain > 0) {
    // Overlap
    for (size_t j = 0; j < num_images; ++j) {
      pix[j] -= (8 - remain);
    }
    temp = vdupq_n_u16(0);
    uint8x8_t pix_val;
    for (size_t j = 0; j < num_images; ++j) {
      pix_val = vld1_u8(pix[j]);
      temp = vaddw_u8(temp, pix_val);
    }
    temp = vaddw_u8(temp, pad);
    vst1_u8(pix[num_images - 1], vqmovn_u16(vshrq_n_u16(temp, exponent)));
  }
#else
  uint16_t s = 0;
  uint8_t pad = num_images / 2;
  for (auto first = images[num_images - 1].begin(),
            last = images[num_images - 1].end();
       first != last; ++first) {
    s = 0;
    for (size_t k = 0; k < num_images; ++k) {
      s += *(pix[k]);
      ++(pix[k]);
    }
    *first = (s + pad) / num_images;
  }
#endif // ANDROID
  imwrite(images[num_images - 1], path + avg_file_name);
  return std::move(images[num_images - 1]);
}

template <size_t num_images, size_t N = 3>
inline matrix<uint8_t, N> avg_folder(const std::string &path_name,
                                     const char *avg_file_name) noexcept {
  return avg_folder<num_images, N>(path_name.c_str(), avg_file_name,
                                   num_images);
}

template <size_t num_images, size_t N = 3>
inline matrix<uint8_t, N> avg_folder(const std::string &path_name,
                                     const char *avg_file_name,
                                     const Margin &margin) noexcept {
  return avg_folder<num_images, N>(path_name.c_str(), avg_file_name, margin);
}

// Sum along a dimension
template <typename U = fp_t, typename M>
decltype(auto) sum(const M &mat, size_t dim = M::order - 1) noexcept {
  static_assert(
      is_matrix<M>() && M::order >= 2,
      "param should be a matrix with order larger than or equal to 2");

  using T = common_type_t<U, typename M::value_type>;
  constexpr size_t N = M::order;

  if (is_empty(mat)) {
    return matrix<T, N - 1>{};
  }

  assert(dim <= M::order);
  // Calculate out dimension
  auto in_desc = mat.descriptor();
  matrix_slice<N - 1> out_desc;
  get_matrix_slice(dim, 0, in_desc, out_desc);

  // Contruct result
  matrix<T, N - 1> ret(out_desc, static_cast<T>(0));

  // Loop through dim
  for (size_t i = 0; i < mat.size(dim); ++i) {
    matrix_ref<typename M::value_type, N - 1> mat_ref{
        out_desc, const_cast<typename M::value_type *>(mat.data())};
    ret += mat_ref;
    out_desc.start += in_desc.strides[dim];
  }

  return ret;
}

template <typename U = fp_t, typename T>
inline decltype(auto) sum(const matrix_ref<T, 1> &mr, size_t dim = 0) noexcept {
  assert(dim == 0);
  ignore(dim);
  using value_t = common_type_t<U, T>;
  return std::accumulate(mr.begin(), mr.end(), static_cast<value_t>(0));
}

template <typename U = fp_t, typename T>
inline decltype(auto) sum(const matrix<T, 1> &m, size_t dim = 0) noexcept {
  assert(dim == 0);
  ignore(dim);
  using value_t = common_type_t<U, T>;
  return std::accumulate(m.begin(), m.end(), static_cast<value_t>(0));
}

template <typename T = fp_t, typename M>
inline decltype(auto) sum_all(const M &mat) noexcept {
  static_assert(is_matrix<M>(), "Operation requires a matrix");
  using value_t = common_type_t<typename M::value_type, T>;
  return std::accumulate(mat.begin(), mat.end(), static_cast<value_t>(0));
}

// filter2
template <typename H, typename M>
decltype(auto) filter2_valid(const H &h, const M &m) noexcept {
  static_assert(is_matrix<H>() && H::order == 2 && is_matrix<M>() &&
                    M::order == 2,
                "Operation requires matrices of order 2");

  using value_t = common_type_t<typename M::value_type, typename H::value_type>;
  if (is_empty(h)) {
    return matrix2<value_t>{m.descriptor(), static_cast<value_t>(0)};
  }

  if (is_empty(m) || (m.rows() <= (h.rows() - 1)) ||
      (m.cols() <= (h.cols() - 1))) {
    return matrix<value_t, 2>{};
  }

  matrix2<value_t> ret(m.rows() - (h.rows() - 1), m.cols() - (h.cols() - 1));
  auto ret_ptr = ret.begin();
  for (size_t i = 0; i < ret.rows(); ++i) {
    for (size_t j = 0; j < ret.cols(); ++j) {
      auto m1 = m(slice{i, i + h.rows() - 1}, slice{j, j + h.cols() - 1});
      *ret_ptr = std::inner_product(h.begin(), h.end(), m1.begin(),
                                    static_cast<value_t>(0));
      ++ret_ptr;
    }
  }
  return ret;
}

template <typename H, typename M>
decltype(auto) filter2_full(const H &h, const M &m) noexcept {
  static_assert(is_matrix<H>() && H::order == 2 && is_matrix<M>() &&
                    M::order == 2,
                "Operation requires matrices of order 2");
  using value_t = common_type_t<typename M::value_type, typename H::value_type>;

  if (is_empty(h)) {
    return matrix2<value_t>{m.descriptor(), static_cast<value_t>(0)};
  }

  if (is_empty(m)) {
    return empty_matrix<value_t, 2>();
  }

  matrix2<value_t> m_pad(m.rows() + 2 * (h.rows() - 1),
                         m.cols() + 2 * (h.cols() - 1));
  // Padding
  m_pad(slice{0, h.rows() - 1}, slice::all) = 0;
  m_pad(slice{m.rows() + (h.rows() - 1)}, slice::all) = 0;
  m_pad(slice::all, slice{0, h.cols() - 1}) = 0;
  m_pad(slice::all, slice{m.cols() + (h.cols() - 1)}) = 0;

  m_pad(slice{h.rows() - 1, m.rows() + (h.rows() - 1) - 1},
        slice{h.cols() - 1, m.cols() + (h.cols() - 1) - 1}) = m;

  matrix2<value_t> ret(m.rows() + (h.rows() - 1), m.cols() + (h.cols() - 1));
  auto ret_ptr = ret.begin();
  for (size_t i = 0; i < ret.rows(); ++i) {
    for (size_t j = 0; j < ret.cols(); ++j) {
      auto m1 = m_pad(slice{i, i + h.rows() - 1}, slice{j, j + h.cols() - 1});
      *ret_ptr = std::inner_product(h.begin(), h.end(), m1.begin(),
                                    static_cast<value_t>(0));
      ++ret_ptr;
    }
  }
  return ret;
}

template <typename H, typename M>
decltype(auto) filter2_same(const H &h, const M &m) noexcept {
  static_assert(is_matrix<H>() && H::order == 2 && is_matrix<M>() &&
                    M::order == 2,
                "Operation requires matrices of order 2");

  using value_t = common_type_t<typename M::value_type, typename H::value_type>;

  if (is_empty(h)) {
    return matrix2<value_t>{m.descriptor(), static_cast<value_t>(0)};
  }

  if (is_empty(m)) {
    return empty_matrix<value_t, 2>();
  }

  matrix2<value_t> m_pad(m.rows() + (h.rows() - 1), m.cols() + (h.cols() - 1));
  // Padding
  m_pad(slice{0, (h.rows() - 1) / 2}, slice::all) = 0;
  m_pad(slice{m.rows() + (h.rows() - 1) / 2}, slice::all) = 0;
  m_pad(slice::all, slice{0, (h.cols() - 1) / 2}) = 0;
  m_pad(slice::all, slice{m.cols() + (h.cols() - 1) / 2}) = 0;

  m_pad(slice{(h.rows() - 1) / 2, m.rows() + (h.rows() - 1) / 2 - 1},
        slice{(h.cols() - 1) / 2, m.cols() + (h.cols() - 1) / 2 - 1}) = m;

  matrix2<value_t> ret(m.rows(), m.cols());
  auto ret_ptr = ret.begin();
  for (size_t i = 0; i < ret.rows(); ++i) {
    for (size_t j = 0; j < ret.cols(); ++j) {
      auto m1 = m_pad(slice{i, i + h.rows() - 1}, slice{j, j + h.cols() - 1});
      *ret_ptr = std::inner_product(h.begin(), h.end(), m1.begin(),
                                    static_cast<value_t>(0));
      ++ret_ptr;
    }
  }
  return ret;
}

template <typename T, typename M>
inline decltype(auto)
filter2_valid(std::initializer_list<std::initializer_list<T>> h,
              const M &m) noexcept {
  static_assert(is_matrix<M>() && M::order == 2,
                "Operation requires matrix of order 2");
  return filter2_valid(matrix2<T>(h), m);
}

template <typename T, typename M>
inline decltype(auto)
filter2_full(std::initializer_list<std::initializer_list<T>> h,
             const M &m) noexcept {
  static_assert(is_matrix<M>() && M::order == 2,
                "Operation requires matrix of order 2");
  return filter2_full(matrix2<T>(h), m);
}

template <typename T, typename M>
inline decltype(auto)
filter2_same(std::initializer_list<std::initializer_list<T>> h,
             const M &m) noexcept {
  static_assert(is_matrix<M>() && M::order == 2,
                "Operation requires matrix of order 2");
  return filter2_same(matrix2<T>(h), m);
}

// findpeaks
// Integral version
template <typename InputIt>
enable_if_t<is_integral<typename std::iterator_traits<InputIt>::value_type>(),
            std::vector<std::pair<
                typename std::iterator_traits<InputIt>::value_type, size_t>>>
findpeaks(InputIt first, InputIt last) noexcept {
  std::vector<std::pair<typename std::iterator_traits<InputIt>::value_type,
                        size_t>> pks;
  InputIt back = first;
  InputIt curr = ++first;
  InputIt next = ++first;
  auto inc_iters = [&]() {
    ++back;
    ++curr;
    ++next;
  };
  size_t n = 1;
  while (next != last) {
    if (*curr < *back) {
      inc_iters();
      ++n;
      continue;
    }
    size_t i = 1;
    while (*curr == *next) {
      inc_iters();
      ++i;
    }
    if (*curr > *next) {
      pks.emplace_back(*curr, n);
    }
    inc_iters();
    n += i;
  }
  return pks;
}

// Floating point version
template <typename InputIt>
enable_if_t<
    is_floating_point<typename std::iterator_traits<InputIt>::value_type>(),
    std::vector<
        std::pair<typename std::iterator_traits<InputIt>::value_type, size_t>>>
findpeaks(InputIt first, InputIt last) noexcept {
  std::vector<std::pair<typename std::iterator_traits<InputIt>::value_type,
                        size_t>> pks;
  InputIt back = first;
  InputIt curr = ++first;
  InputIt next = ++first;
  auto inc_iters = [&]() {
    ++back;
    ++curr;
    ++next;
  };
  size_t n = 1;
  while (next != last) {
    if (*curr < *back || almost_equal(*curr, *back)) {
      inc_iters();
      ++n;
      continue;
    }
    size_t i = 1;
    while (almost_equal(*curr, *next)) {
      inc_iters();
      ++i;
    }
    if (*curr > *next) {
      pks.emplace_back(*curr, n);
    }
    inc_iters();
    n += i;
  }
  return pks;
}

template <typename V>
enable_if_t<is_matrix<V>() && V::order == 1 &&
                is_arithmetic<typename V::value_type>(),
            std::vector<std::pair<typename V::value_type, size_t>>>
findpeaks(const V &v, sort_t st,
          typename V::value_type thr =
              std::numeric_limits<typename V::value_type>::max()) noexcept {
  auto pks = findpeaks(v.begin(), v.end());
  using T = typename V::value_type;
  using P = typename std::pair<T, size_t>;
  switch (st) {
  case sort_t::descend:
    std::sort(pks.begin(), pks.end(),
              [](const P &a, const P &b) { return a.first > b.first; });
    break;
  case sort_t::ascend:
    std::sort(pks.begin(), pks.end(),
              [](const P &a, const P &b) { return a.first < b.first; });
    break;
  case sort_t::none:
    // Do nothing
    break;
  }

  // Erase remove idiom
  pks.erase(std::remove_if(pks.begin(), pks.end(), [&](const P &p) {
    return p.first < thr;
  }), pks.end());
  return pks;
}

template <typename T> struct g_slice {
  T start;
  T end;
  T inc;
  explicit g_slice(T s, T e, T n = static_cast<T>(1))
      : start(s), end(e), inc(n) {
    assert(s <= e);
  }
};

template <typename T>
decltype(auto) meshgrid(const g_slice<T> &gs1, const g_slice<T> &gs2) noexcept {
  static_assert(is_arithmetic<T>(), "Operation requires arithmetic type");

  size_t w = (gs1.end - gs1.start) / gs1.inc + 1;
  size_t h = (gs2.end - gs2.start) / gs2.inc + 1;
  matrix2<T> x(h, w);
  auto iter = x.begin();
  T val = gs1.start;
  T inc = gs1.inc;
  for (size_t i = 0; i < h; ++i) {
    val = gs1.start;
    for (size_t j = 0; j < w; ++j) {
      *iter = val;
      val += inc;
      ++iter;
    }
  }

  matrix2<T> y(h, w);
  iter = y.begin();
  val = gs2.start;
  inc = gs2.inc;
  for (size_t i = 0; i < h; ++i) {
    for (size_t j = 0; j < w; ++j) {
      *iter = val;
      ++iter;
    }
    val += inc;
  }

  return std::make_pair(std::move(x), std::move(y));
}

template <typename T, size_t N, typename... Args>
inline matrix<T, N> zeros(Args... args) noexcept {
  matrix<T, N> m(args...);
  std::fill(m.begin(), m.end(), static_cast<T>(0));
  return m;
}

template <typename T, size_t N, typename... Args>
inline matrix<T, N> ones(Args... args) noexcept {
  matrix<T, N> m(args...);
  std::fill(m.begin(), m.end(), static_cast<T>(1));
  return m;
}

template <typename T = fp_t>
inline matrix2<T> fspecial_gaussian(const std::pair<int, int> &siz,
                                    T std) noexcept {
  static_assert(is_floating_point<T>(),
                "Operation requires floating point type");

  assert(siz.first >= 1 && siz.second >= 1);
  auto siz1 = (siz.first - 1) / 2;
  auto siz2 = (siz.second - 1) / 2;

  matrix2<T> h(siz.first, siz.second);
  auto h_iter = h.begin();
  auto sigma = 2 * std * std;
  for (auto x = -siz1; x <= siz1; ++x) {
    for (auto y = -siz2; y <= siz2; ++y) {
      *h_iter = std::exp(-(x * x + y * y) / sigma);
      ++h_iter;
    }
  }

  auto sumh = sum_all(h);
  h /= sumh;
  return h;
}

template <typename T>
inline enable_if_t<is_floating_point<T>(), matrix2<T>>
fspecial_gaussian(int siz, const T &std) noexcept {
  return fspecial_gaussian(std::make_pair(siz, siz), std);
}

template <typename T>
inline enable_if_t<is_floating_point<T>(), matrix2<T>>
fspecial_average(const std::pair<int, int> &siz) noexcept {
  matrix2<T> h = ones<T, 2>(siz.first, siz.second);
  T sumh = h.size(0) * h.size(1);
  h /= sumh;
  return h;
}

// rgb2ycbcr
// Convert RGB color space to YCbCr color space
template <typename T = fp_t, typename M>
decltype(auto) rgb2ycbcr(const M &m) noexcept {
  static_assert(is_matrix<M>() && M::order == 3 && is_floating_point<T>(),
                "Operation requires matrix of order 3 and floating point type");
  assert(m.size(2) == 3);
  using value_t = common_type_t<T, typename M::value_type>;
  matrix3<value_t> ret(m.size(0), m.size(1), m.size(2));
  value_t r, g, b;
  auto p = ret.begin();
  for (auto first = m.begin(), last = m.end(); first != last; ++first) {
    r = *first / 255.;
    g = *++first / 255.;
    b = *++first / 255.;
    *p = 65.481 * r + 128.553 * g + 24.966 * b + 16;
    *++p = -37.797 * r - 74.203 * g + 112 * b + 128;
    *++p = 112 * r - 93.786 * g - 18.214 * b + 128;
    ++p;
  }
  return ret;
}

// rgb2y
// Convert RGB color space to YCbCr color space. This is used in case we lack of
// memory and only want Y channel
template <typename T = fp_t, typename M>
decltype(auto) rgb2y(const M &m) noexcept {
  static_assert(is_matrix<M>() && M::order == 3 && is_floating_point<T>(),
                "Operation requires matrix of order 3 and floating point type");
  assert(m.size(2) == 3);
  using value_t = common_type_t<T, typename M::value_type>;
  matrix2<value_t> ret(m.size(0), m.size(1));
  auto p = ret.begin();
  for (auto first = m.begin(), last = m.end(); first != last; ++first) {
    *p += 65.481 * (*first / 255.);
    *p += 128.553 * (*++first / 255.);
    *p += 24.966 * (*++first / 255.) + 16;
    ++p;
  }
  return ret;
}

template <typename T = fp_t, typename M>
decltype(auto) rgb2gray(const M &m) noexcept {
  static_assert(is_matrix<M>() && M::order == 3 && is_floating_point<T>(),
                "Operation requires matrix of order 3 and floating point type");
  assert(m.size(2) == 3);
  using value_t = common_type_t<T, typename M::value_type>;
  matrix2<value_t> ret(m.size(0), m.size(1));
  matrix2<value_t> t = {
      {1.0, 0.956, 0.621}, {1.0, -0.272, -0.647}, {1.0, -1.106, 1.703}};
  auto det = t(0, 0) * (t(1, 1) * t(2, 2) - t(2, 1) * t(1, 2)) +
             t(0, 1) * (t(1, 2) * t(2, 0) - t(2, 2) * t(1, 0)) +
             t(0, 2) * (t(1, 0) * t(2, 1) - t(2, 0) * t(1, 1));
  value_t coef[3] = {(t(1, 1) * t(2, 2) - t(2, 1) * t(1, 2)) / det,
                     (t(0, 2) * t(2, 1) - t(2, 2) * t(0, 1)) / det,
                     (t(0, 1) * t(1, 2) - t(1, 1) * t(0, 2)) / det};
  auto p = ret.begin();
  for (auto first = m.begin(), last = m.end(); first != last; ++first) {
    *p += coef[0] * (*first / 255.);
    *p += coef[1] * (*++first / 255.);
    *p += coef[2] * (*++first / 255.);
    *p = std::min(std::max(*p, static_cast<value_t>(0)),
                  static_cast<value_t>(1));
    ++p;
  }
  return ret;
}

// mat2gray
// Rescale intensities between 0 and 255, inclusive. This version is for
// floating point types
template <typename M>
enable_if_t<is_matrix<M>() && is_floating_point<typename M::value_type>(),
            matrix<typename M::value_type, M::order>>
mat2gray(const M &m) noexcept {
  auto extrema = imtoolbox::minmax_element(m.begin(), m.end());
  auto min_m = *extrema.first;
  auto max_m = *extrema.second;

  auto ret = m.clone();
  if (almost_equal(max_m, min_m)) {
    // Rescale value to [0, 255]
    for (auto first = ret.begin(), last = ret.end(); first != last; ++first) {
      *first = 255 * (*first / max_m);
    }
  } else {
    for (auto first = ret.begin(), last = ret.end(); first != last; ++first) {
      *first = 255 * (*first - min_m) / (max_m - min_m);
    }
  }
  return ret;
}

// mat2gray
// Rescale intensities between 0 and 255, inclusive. This version is for
// integral types
template <typename M>
enable_if_t<is_matrix<M>() && is_integral<typename M::value_type>(),
            matrix<typename M::value_type, M::order>>
mat2gray(const M &m) noexcept {
  auto extrema = imtoolbox::minmax_element(m.begin(), m.end());
  auto min_m = *extrema.first;
  auto max_m = *extrema.second;

  auto ret = m.clone();
  if (max_m == min_m) {
    // Rescale value to [0, 255]
    for (auto first = ret.begin(), last = ret.end(); first != last; ++first) {
      *first = 255 * (*first / max_m);
    }
  } else {
    for (auto first = ret.begin(), last = ret.end(); first != last; ++first) {
      *first = 255 * (*first - min_m) / (max_m - min_m);
    }
  }
  return ret;
}

// f should map value between 0 and 255
template <typename T = fp_t, typename ForwardIterator, typename F>
inline std::vector<T> imhist(ForwardIterator first, ForwardIterator last,
                             int num_bins, F f) noexcept {
  std::vector<T> p(num_bins, 0);
  while (first != last) {
    size_t v = std::round(f(*first)) * num_bins / 256;
    p[v] += 1;
    ++first;
  }
  return p;
}

// getpdf for floating point
template <typename T = fp_t, typename M>
enable_if_t<is_floating_point<typename M::value_type>(),
            std::tuple<std::vector<common_type_t<T, typename M::value_type>>,
                       common_type_t<T, typename M::value_type>,
                       common_type_t<T, typename M::value_type>>>
getpdf(const M &m, int num_bins) noexcept {
  using value_t = common_type_t<T, typename M::value_type>;
  auto extrema = imtoolbox::minmax_element(m.begin(), m.end(), [](auto x) {
    return !std::isnan(x) && !std::isinf(x);
  });

  if (extrema.first == m.end() && extrema.second == m.end()) {
    return std::make_tuple(std::vector<value_t>(), 0, 0);
  }

  value_t min_m = *extrema.first;
  value_t max_m = *extrema.second;

  auto p = imhist<value_t>(m.begin(), m.end(), num_bins, [&](auto x) {
    return 255 * (x - min_m) / (max_m - min_m);
  });
  auto sump = std::accumulate(p.begin(), p.end(), 0);
  for (auto &x : p) {
    x /= sump;
  }
  return std::make_tuple(std::move(p), max_m, min_m);
}

// getpdf for integral types
template <typename T = fp_t, typename M>
enable_if_t<is_integral<typename M::value_type>(),
            std::tuple<std::vector<common_type_t<T, typename M::value_type>>,
                       common_type_t<T, typename M::value_type>,
                       common_type_t<T, typename M::value_type>>>
getpdf(const M &m, int num_bins) noexcept {
  using value_t = common_type_t<T, typename M::value_type>;
  auto extrema = imtoolbox::minmax_element(m.begin(), m.end());

  // Don't care about nan and inf
  value_t min_m = *extrema.first;
  value_t max_m = *extrema.second;

  if (max_m == min_m) {
    return std::make_tuple(std::vector<value_t>(), 0, 0);
  }
  auto p = imhist<value_t>(m.begin(), m.end(), num_bins, [&](auto x) {
    return static_cast<value_t>(255) * (x - min_m) / (max_m - min_m);
  });
  auto sump = std::accumulate(p.begin(), p.end(), 0);
  for (auto &x : p) {
    x /= sump;
  }
  return std::make_tuple(std::move(p), max_m, min_m);
}

template <typename T>
matrix2<T> calc_full_obj_criteria_matrix(int n, const std::vector<T> &omega,
                                         const std::vector<T> &mu,
                                         T mu_t) noexcept {
  assert(n <= 2 && omega.size() == mu.size());
  auto num_bins = omega.size();
  matrix2<T> result(num_bins, ((n == 1) ? 1 : num_bins));
  if (n == 1) {
    auto p = result.begin();
    for (size_t i = 0; i < num_bins; ++i) {
      auto term = mu_t * omega[i] - mu[i];
      *p = (term * term) / (omega[i] * (1 - omega[i]));
      ++p;
    }
  } else if (n == 2) {
    auto p = result.begin();
    for (size_t i = 0; i < num_bins; ++i) {
      for (size_t j = 0; j < num_bins; ++j) {
        auto omega0 = omega[i];
        auto mu_0_t = mu_t - mu[i] / omega[i];
        auto omega1 = omega[j] - omega[i];
        auto mu_1_t = mu_t - (mu[j] - mu[i]) / omega1;
        auto term1 = omega0 * (mu_0_t * mu_0_t);
        auto term2 = omega1 * (mu_1_t * mu_1_t);
        auto omega2 = 1 - (omega0 + omega1);
        auto term3 = omega0 * mu_0_t + omega1 * mu_1_t;
        (term3 *= term3) /= omega2;
        *p = term1 + term2 + term3;
        ++p;
      }
    }
  }
  return result;
}

// multithresh
template <typename M>
enable_if_t<is_matrix<M>(), std::vector<typename M::value_type>>
multithresh(const M &m, size_t n) noexcept {
  assert(n <= 2);
  constexpr int num_bins = 256;
  using value_t = typename M::value_type;
  std::vector<value_t> p;
  value_t max_m, min_m;
  std::tie(p, max_m, min_m) = getpdf<value_t>(m, num_bins);

  if (is_floating_point<value_t>() && p.size() == 0) {
    println_e("degenerated input");
    return std::vector<value_t>{};
  }

  auto omega = p;
  std::partial_sum(omega.begin(), omega.end(), omega.begin());

  auto mu = p;
  for (size_t i = 0; i < p.size(); ++i) {
    mu[i] *= (i + 1);
  }
  std::partial_sum(mu.begin(), mu.end(), mu.begin());
  auto mu_t = mu[mu.size() - 1];

  auto sigma_b_squared = calc_full_obj_criteria_matrix(n, omega, mu, mu_t);

  auto max_sigma_b_squared = imtoolbox::max_element(
      sigma_b_squared.begin(), sigma_b_squared.end(),
      [](auto x) { return !std::isnan(x) && !std::isinf(x); });

  if (max_sigma_b_squared.second == num_bins * num_bins) {
    println_e("degenerated input or no convergence");
    return std::vector<value_t>{};
  }

  auto max_r = max_sigma_b_squared.second / num_bins;
  auto max_c = max_sigma_b_squared.second % num_bins;

  std::vector<value_t> thresh;
  if (n == 1) {
    thresh.push_back((max_r + max_c) / static_cast<value_t>(2));
  } else if (n == 2) {
    thresh.push_back(max_r);
    thresh.push_back(max_c);
  }

  for (auto &t : thresh) {
    t = min_m + t / 255.0 * (max_m - min_m);
  }

  sort(thresh.begin(), thresh.end());
  return thresh;
}

// pctl_hist_thresh
template <typename T = fp_t, typename U>
enable_if_t<is_floating_point<T>(), T> pctl_hist_thresh(const std::vector<U> &h,
                                                        T p) noexcept {
  assert(p <= 100);
  T thr = std::accumulate(h.begin(), h.end(), static_cast<T>(0)) * p;
  T s = 0;
  T idx = 0;
  for (size_t i = 0; i < h.size(); ++i) {
    s += h[i];
    if (s >= thr) {
      idx = i;
      break;
    }
  }
  return idx;
}

template <typename M, typename S>
enable_if_t<is_matrix<M>() && is_matrix<S>() && M::order == 2 &&
                S::order == 2 && is_same<typename M::value_type, logical>() &&
                is_same<typename S::value_type, logical>(),
            matrix2<logical>>
binary_dilate(const M &in, const S &strel) noexcept {
  if (is_empty(in)) {
    return empty_matrix<logical, 2>();
  }

  if (is_empty(strel)) {
    return zeros<logical, 2>(in.descriptor());
  }

  std::vector<std::pair<size_t, size_t>> strel_on;
  {
    auto p = strel.begin();
    for (size_t i = 0; i < strel.size(0); ++i) {
      for (size_t j = 0; j < strel.size(1); ++j) {
        if (1 == *p) {
          strel_on.emplace_back(i, j);
        }
        ++p;
      }
    }
  }

  matrix2<logical> out(in.size(0), in.size(1));
  auto p = out.begin();
  auto r_in = in.size(0);
  auto c_in = in.size(1);
  auto r_strel = strel.size(0);
  auto c_strel = strel.size(1);
  for (size_t r = 0; r < r_in; ++r) {
    for (size_t c = 0; c < c_in; ++c) {
      logical out_pixel = 0;
      for (size_t i = 0; i < strel_on.size() && 0 == out_pixel; ++i) {
        auto m = r + strel_on[i].first;
        auto n = c + strel_on[i].second;
        if (m >= (r_strel / 2) && n >= (c_strel / 2) &&
            (m - r_strel / 2) < r_in && (n - c_strel / 2) < c_in) {
          out_pixel = in(m - r_strel / 2, n - c_strel / 2);
        }
      }
      *p = out_pixel;
      ++p;
    }
  }
  return out;
}

template <typename M, typename S>
enable_if_t<is_matrix<M>() && is_matrix<S>() && M::order == 2 &&
                S::order == 2 && is_same<typename M::value_type, logical>() &&
                is_same<typename S::value_type, logical>(),
            matrix2<logical>>
binary_dilate(M &&in, const S &strel) noexcept {
  if (is_empty(in)) {
    return empty_matrix<logical, 2>();
  }

  if (is_empty(strel)) {
    return zeros<logical, 2>(in.descriptor());
  }

  std::vector<std::pair<size_t, size_t>> strel_on;
  {
    auto p = strel.begin();
    for (size_t i = 0; i < strel.size(0); ++i) {
      for (size_t j = 0; j < strel.size(1); ++j) {
        if (1 == *p) {
          strel_on.emplace_back(i, j);
        }
        ++p;
      }
    }
  }

  matrix2<logical> out = std::move(in);
  auto p = out.begin();
  auto r_in = in.size(0);
  auto c_in = in.size(1);
  auto r_strel = strel.size(0);
  auto c_strel = strel.size(1);
  for (size_t r = 0; r < r_in; ++r) {
    for (size_t c = 0; c < c_in; ++c) {
      if (0 == *p) {
        logical out_pixel = 0;
        for (size_t i = 0; i < strel_on.size() && 0 == out_pixel; ++i) {
          auto m = r + strel_on[i].first;
          auto n = c + strel_on[i].second;
          if (m >= (r_strel / 2) && n >= (c_strel / 2) &&
              (m - r_strel / 2) < r_in && (n - c_strel / 2) < c_in &&
              1 == out(m - r_strel / 2, n - c_strel / 2)) {
            out_pixel = 2;
          }
        }
        *p = out_pixel;
      }
      ++p;
    }
  }
  for (auto first = out.begin(), last = out.end(); first != last; ++first) {
    if (2 == *first) {
      *first = 1;
    }
  }
  return out;
}

template <typename M, typename S>
enable_if_t<is_matrix<M>() && is_matrix<S>() && M::order == 2 &&
                S::order == 2 && is_same<typename M::value_type, logical>() &&
                is_same<typename S::value_type, logical>(),
            matrix2<logical>>
binary_erode(const M &in, const S &strel) noexcept {
  if (is_empty(in)) {
    return empty_matrix<logical, 2>();
  }

  if (is_empty(strel)) {
    return ones<logical, 2>(in.descriptor());
  }

  std::vector<std::pair<size_t, size_t>> strel_on;
  {
    auto p = strel.begin();
    for (size_t i = 0; i < strel.size(0); ++i) {
      for (size_t j = 0; j < strel.size(1); ++j) {
        if (1 == *p) {
          strel_on.emplace_back(i, j);
        }
        ++p;
      }
    }
  }

  matrix2<logical> out(in.size(0), in.size(1));
  auto p = out.begin();
  auto r_in = in.size(0);
  auto c_in = in.size(1);
  auto r_strel = strel.size(0);
  auto c_strel = strel.size(1);
  for (size_t r = 0; r < r_in; ++r) {
    for (size_t c = 0; c < c_in; ++c) {
      logical out_pixel = 1;
      for (size_t i = 0; i < strel_on.size() && 1 == out_pixel; ++i) {
        auto m = r + strel_on[i].first;
        auto n = c + strel_on[i].second;
        if (m >= (r_strel / 2) && n >= (c_strel / 2) &&
            (m - r_strel / 2) < r_in && (n - c_strel / 2) < c_in) {
          out_pixel = in(m - r_strel / 2, n - c_strel / 2);
        }
      }
      *p = out_pixel;
      ++p;
    }
  }
  return out;
}

template <typename M, typename S>
enable_if_t<is_matrix<M>() && is_matrix<S>() && M::order == 2 &&
                S::order == 2 && is_same<typename M::value_type, logical>() &&
                is_same<typename S::value_type, logical>(),
            matrix2<typename M::value_type>>
binary_erode(M &&in, const S &strel) noexcept {
  using value_t = typename M::value_type;
  if (is_empty(in)) {
    return empty_matrix<value_t, 2>();
  }

  if (is_empty(strel)) {
    return ones<logical, 2>(in.descriptor());
  }

  std::vector<std::pair<size_t, size_t>> strel_on;
  {
    auto p = strel.begin();
    for (size_t i = 0; i < strel.size(0); ++i) {
      for (size_t j = 0; j < strel.size(1); ++j) {
        if (1 == *p) {
          strel_on.emplace_back(i, j);
        }
        ++p;
      }
    }
  }

  matrix2<value_t> out = std::move(in);
  auto p = out.begin();
  auto r_in = in.size(0);
  auto c_in = in.size(1);
  auto r_strel = strel.size(0);
  auto c_strel = strel.size(1);
  for (size_t r = 0; r < r_in; ++r) {
    for (size_t c = 0; c < c_in; ++c) {
      if (1 == *p) {
        uint32_t out_pixel = 1;
        for (size_t i = 0; i < strel_on.size() && 1 == out_pixel; ++i) {
          auto m = r + strel_on[i].first;
          auto n = c + strel_on[i].second;
          if (m >= (r_strel / 2) && n >= (c_strel / 2) &&
              (m - r_strel / 2) < r_in && (n - c_strel / 2) < c_in &&
              0 == out(m - r_strel / 2, n - c_strel / 2)) {
            out_pixel = 2;
          }
        }
        *p = out_pixel;
      }
      ++p;
    }
  }

  for (auto first = out.begin(), last = out.end(); first != last; ++first) {
    if (2 == *first) {
      *first = 0;
    }
  }
  return out;
}

struct structure_element {
  structure_element() = default;
  structure_element(structure_element &&) = default;
  structure_element &operator=(structure_element &&) = default;
  structure_element(const structure_element &) = delete;
  structure_element &operator=(const structure_element &) = delete;

  std::vector<matrix2<logical>> decomposition;
};

inline matrix2<logical> make_periodic_line(int p, int v1, int v2) noexcept {
  assert(p >= 0);
  std::vector<int> r(2 * p + 1);
  std::vector<int> c(2 * p + 1);
  int m = 0;
  int n = 0;
  for (size_t i = 0; i < r.size(); ++i) {
    r[i] = (i - p) * v1;
    if (std::abs(r[i]) > m) {
      m = std::abs(r[i]);
    }
    c[i] = (i - p) * v2;
    if (std::abs(c[i]) > n) {
      n = std::abs(c[i]);
    }
  }
  matrix2<logical> result(2 * m + 1, 2 * n + 1);
  for (size_t i = 0; i < r.size(); ++i) {
    result(r[i] + m, c[i] + n) = 1;
  }
  return result;
}

inline structure_element strel_disk(int r, int n = 4) noexcept {
  assert(r >= 0);
  std::vector<std::pair<int, int>> v;
  switch (n) {
  case 4:
    v = {{1, 0}, {1, 1}, {0, 1}, {-1, 1}};
    break;
  case 6:
    v = {{1, 0}, {1, 2}, {2, 1}, {0, 1}, {-1, 2}, {-2, 1}};
    break;
  case 8:
    v = {{1, 0}, {2, 1}, {1, 1}, {1, 2}, {0, 1}, {-1, 2}, {-1, 1}, {-2, 1}};
    break;
  default:
    println_w("Invalid parameter n, assume n = 4");
    v = {{1, 0}, {1, 1}, {0, 1}, {-1, 1}};
    break;
  }
  constexpr double pi = 3.14159265358979323846;
  auto theta = pi / (2 * n);
  auto k = 2 * r / ((cos(theta) + 1) / sin(theta));
  structure_element se;
  for (int q = 0; q < n; ++q) {
    auto rp =
        floor(k / sqrt(v[q].first * v[q].first + v[q].second * v[q].second));
    se.decomposition.emplace_back(
        make_periodic_line(rp, v[q].first, v[q].second));
  }
  int max_radius = 1;
  for (size_t i = 0; i < se.decomposition.size(); ++i) {
    max_radius += se.decomposition[i].size(0) - 1;
  }
  int len = 2 * (r - (max_radius / 2) - 1) + 1;

  if (len >= 3) {
    se.decomposition.emplace_back(ones<logical, 2>(1, len));
    se.decomposition.emplace_back(ones<logical, 2>(len, 1));
  }
  return se;
}

template <typename M>
enable_if_t<is_matrix<M>() && M::order == 2, matrix2<logical>>
binary_erode(const M &in, const structure_element &strel) noexcept {
  static_assert(is_same<typename M::value_type, logical>(),
                "Require logical types");
  auto out = in.clone();
  for (size_t i = 0; i < strel.decomposition.size(); ++i) {
    out = binary_erode(std::move(out), strel.decomposition[i]);
  }
  return out;
}

template <typename M>
enable_if_t<is_matrix<M>() && M::order == 2 &&
                is_same<typename M::value_type, logical>(),
            matrix2<logical>>
binary_erode(M &&in, const structure_element &strel) noexcept {
  auto out = std::move(in);
  for (size_t i = 0; i < strel.decomposition.size(); ++i) {
    out = binary_erode(std::move(out), strel.decomposition[i]);
  }
  return out;
}

template <typename M>
enable_if_t<is_matrix<M>() && M::order == 2 &&
                is_same<typename M::value_type, logical>(),
            matrix2<logical>>
binary_dilate(const M &in, const structure_element &strel) noexcept {
  auto out = in.clone();
  for (size_t i = 0; i < strel.decomposition.size(); ++i) {
    out = binary_dilate(std::move(out), strel.decomposition[i]);
  }
  return out;
}

template <typename M>
enable_if_t<is_matrix<M>() && M::order == 2 &&
                is_same<typename M::value_type, logical>(),
            matrix2<logical>>
binary_dilate(M &&in, const structure_element &strel) noexcept {
  auto out = std::move(in);
  for (size_t i = 0; i < strel.decomposition.size(); ++i) {
    out = binary_dilate(std::move(out), strel.decomposition[i]);
  }
  return out;
}

template <typename M>
enable_if_t<is_matrix<M>() && M::order == 2 &&
                is_same<typename M::value_type, logical>(),
            matrix2<logical>>
binary_open(const M &in, const structure_element &strel) noexcept {
  return binary_dilate(binary_erode(in, strel), strel);
}

template <typename M>
enable_if_t<is_matrix<M>() && M::order == 2 &&
                is_same<typename M::value_type, logical>(),
            matrix2<logical>>
binary_open(M &&in, const structure_element &strel) noexcept {
  return binary_dilate(binary_erode(std::move(in), strel), strel);
}

template <typename M>
enable_if_t<is_matrix<M>() && M::order == 2 &&
                is_same<typename M::value_type, logical>(),
            matrix2<logical>>
binary_close(const M &in, const structure_element &strel) noexcept {
  return binary_erode(binary_dilate(in, strel), strel);
}

template <typename M>
enable_if_t<is_matrix<M>() && M::order == 2 &&
                is_same<typename M::value_type, logical>(),
            matrix2<logical>>
binary_close(M &&in, const structure_element &strel) noexcept {
  return binary_erode(binary_dilate(std::move(in), strel), strel);
}

// bwlabel
template <typename M> matrix2<int> bwlabel(const M &in) noexcept {
  static_assert(is_matrix<M>() && M::order == 2,
                "Operation requires 2D matrix");
  std::vector<std::pair<size_t, size_t>> cc = {
      {0, 0}, {0, 1}, {0, 2}, {1, 0}, {1, 2}, {2, 0}, {2, 1}, {2, 2}};

  int label{1};
  matrix2<int> out(in.descriptor(), -1);
  std::queue<std::pair<size_t, size_t>> segment;
  for (size_t i = 0; i < in.size(0); ++i) {
    for (size_t j = 0; j < in.size(1); ++j) {
      if (out(i, j) >= 0) {
        continue;
      } else if (in(i, j) == 0) {
        out(i, j) = 0;
        continue;
      }
      out(i, j) = label;
      segment.emplace(i, j);
      while (!segment.empty()) {
        auto idx = segment.front();
        for (size_t k = 0; k < cc.size(); ++k) {
          auto m = idx.first + cc[k].first;
          auto n = idx.second + cc[k].second;
          if (m >= 1 && n >= 1 && m - 1 < in.size(0) && n - 1 < in.size(1) &&
              out(m - 1, n - 1) < 0 && in(m - 1, n - 1) != 0) {
            out(m - 1, n - 1) = label;
            segment.emplace(m - 1, n - 1);
          }
        }
        segment.pop();
      }
      ++label;
    }
  }
  return out;
}

template <typename M>
matrix2<int> bwlabel(const M &in, std::vector<Rect> &rects) noexcept {
  static_assert(is_matrix<M>() && M::order == 2,
                "Operation requires 2D matrix");
  std::vector<std::pair<size_t, size_t>> cc = {
      {0, 0}, {0, 1}, {0, 2}, {1, 0}, {1, 2}, {2, 0}, {2, 1}, {2, 2}};

  int label{1};
  matrix2<int> out(in.descriptor(), -1);
  std::queue<std::pair<size_t, size_t>> segment;
  for (size_t i = 0; i < in.size(0); ++i) {
    for (size_t j = 0; j < in.size(1); ++j) {
      if (out(i, j) >= 0) {
        continue;
      } else if (in(i, j) == 0) {
        out(i, j) = 0;
        continue;
      }
      out(i, j) = label;
      size_t y1 = i;
      size_t y2 = i;
      size_t x1 = j;
      size_t x2 = j;
      segment.emplace(i, j);
      while (!segment.empty()) {
        auto idx = segment.front();
        for (size_t k = 0; k < cc.size(); ++k) {
          auto m = idx.first + cc[k].first;
          auto n = idx.second + cc[k].second;
          if (m >= 1 && n >= 1 && m - 1 < in.size(0) && n - 1 < in.size(1) &&
              out(m - 1, n - 1) < 0 && in(m - 1, n - 1) != 0) {
            out(m - 1, n - 1) = label;
            y1 = std::min(y1, m - 1);
            y2 = std::max(y2, m - 1);
            x1 = std::min(x1, n - 1);
            x2 = std::max(x2, n - 1);
            segment.emplace(m - 1, n - 1);
          }
        }
        segment.pop();
      }
      rects.emplace_back(x1, x2, y1, y2);
      ++label;
    }
  }
  return out;
}

template <typename M1, typename T, typename M2>
inline decltype(auto) scale_and_add(const M1 &m1, const T &t,
                                    const M2 &m2) noexcept {
  static_assert(is_matrix<M1>() && is_matrix<M2>(),
                "Operation requires matrices");
  assert(same_extents(m1.descriptor(), m2.descriptor()));
  using value_t =
      common_type_t<typename M1::value_type, typename M2::value_type, T>;
  matrix<value_t, M1::order> result(m1.descriptor());
  std::transform(m1.begin(), m1.end(), m2.begin(), result.begin(),
                 [&](value_t x, value_t y) { return x * t + y; });
  return result;
}

template <typename M, typename V>
void gaussian_elimination(M &A, V &b) noexcept {
  static_assert(is_matrix2d<M>(), "Operation requires matrix type");
  static_assert(is_same<typename M::value_type, typename V::value_type>(),
                "A and b should be same types");
  static_assert(is_floating_point<typename M::value_type>(),
                "Operation requires floating point");
  assert(A.rows() == A.cols() && A.rows() == b.size());
  using T = typename M::value_type;
  const size_t n = A.rows();

  for (size_t j = 0; j < n; ++j) {
    size_t pivot_row = j;
    // look for suitable pivot
    for (size_t k = j + 1; k < n; ++k) {
      if (std::abs(A(k, j)) > std::abs(A(pivot_row, j))) {
        pivot_row = k;
      }
    }

    // Swap the rows if we found a better pivot
    if (pivot_row != j) {
      A.swap_rows(j, pivot_row);
      std::swap(b(j), b(pivot_row));
    }

    // Elimination
    for (size_t i = j + 1; i < n; ++i) {
      const T pivot = A(j, j);
      if (almost_equal(pivot, static_cast<T>(0))) {
        // Singular matrix, elimination done
        println_w("Singular matrix in elimination step");
        return;
      }
      const T mult = A(i, j) / pivot;
      A(i, slice{j}) = scale_and_add(A(j, slice{j}), -mult, A(i, slice{j}));
      b(i) -= mult * b(j);
    }
  }
}

template <typename V1, typename V2>
inline decltype(auto) dot_product(const V1 &v1, const V2 &v2) noexcept {
  static_assert(is_matrix<V1>() && is_matrix<V2>(),
                "Operation requires matrices");
  assert(same_size(v1.descriptor(), v2.descriptor()));
  using value_t =
      common_type_t<typename V1::value_type, typename V2::value_type>;
  return std::inner_product(v1.begin(), v1.end(), v2.begin(),
                            static_cast<value_t>(0));
}

template <typename M, typename V>
decltype(auto) back_substitution(const M &A, const V &b) noexcept {
  static_assert(is_matrix2d<M>(), "Operation requires matrix");
  static_assert(is_same<typename M::value_type, typename V::value_type>(),
                "A and b should be same types");
  static_assert(is_floating_point<typename M::value_type>(),
                "Operation requires floating point");
  assert(A.rows() == A.cols() && A.rows() == b.size());
  using T = typename M::value_type;
  const size_t n = A.rows();
  vector<T> x(b.descriptor(), static_cast<T>(0));
  for (size_t i = n; i > 0; --i) {
    T s = b(i - 1) - dot_product(A(i - 1, slice{i - 1}), x(slice{i - 1}));
    T m = A(i - 1, i - 1);
    if (!almost_equal(m, static_cast<T>(0))) {
      x(i - 1) = s / m;
    } else {
      println_w("Singular matrix in back_substitution");
      if (!almost_equal(s, static_cast<T>(0))) {
        // Value is garbage
        x(i - 1) = std::numeric_limits<T>::infinity();
      }
    }
  }
  return x;
}

template <typename T = fp_t, typename ForwardIterator1,
          typename ForwardIterator2>
void build_matrix_vector(ForwardIterator1 first1, ForwardIterator1 last1,
                         ForwardIterator2 first2, size_t k, matrix2<T> &A,
                         vector<T> &b) noexcept {
  size_t n = 2 * k;
  std::vector<T> x_terms(n + 1);
  while (first1 != last1) {
    T x_prod = 1;
    for (size_t i = 0; i <= n; ++i) {
      x_terms[i] += x_prod;
      x_prod *= *first1;
    }
    T y_prod = *first2;
    for (size_t i = 0; i <= k; ++i) {
      b(i) += y_prod;
      y_prod *= *first1;
    }
    ++first1;
    ++first2;
  }
  for (size_t i = 0; i <= k; ++i) {
    for (size_t j = 0; j <= k; ++j) {
      A(i, j) = x_terms[i + j];
    }
  }
}

template <typename X, typename Y>
inline decltype(auto) fit_poly(const X &x, const Y &y, size_t k) noexcept {
  assert(x.size() == y.size() && x.size() > k);
  using T = common_type_t<typename X::value_type, typename Y::value_type, fp_t>;
  matrix2<T> A(k + 1, k + 1);
  vector<T> b(k + 1);
  build_matrix_vector(x.begin(), x.end(), y.begin(), k, A, b);
  gaussian_elimination(A, b);
  return back_substitution(A, b);
}

template <typename Y>
inline decltype(auto) fit_array(size_t x_min, size_t x_max, const Y &y,
                                size_t k) noexcept {
  assert(x_max - x_min + 1 == y.size());
  using T = common_type_t<typename Y::value_type, fp_t>;
  std::vector<T> x;
  for (size_t i = x_min; i <= x_max; ++i) {
    x.push_back(i);
  }
  return fit_poly(x, y, k);
}

} // namespace imtoolbox
#endif
