#include "tlc.h"
#include "imtoolbox.h"
#include <sys/stat.h>

namespace tlc {
using namespace imtoolbox;
template <typename T>
decltype(auto) detect_lines(const matrix2<T> &m) noexcept {
  auto sigma = static_cast<T>(4);
  auto h_size = static_cast<int>(6 * sigma + 1);

  auto dX =
      filter2_valid(fspecial_gaussian({h_size, 1}, sigma),
                    filter2_valid(fspecial_gaussian({1, h_size}, sigma), m));

  dX = filter2_valid({{1, -2, 1}}, dX);

  auto dx = sum<T>(dX, 0);

  auto pks = findpeaks(dx, sort_t::descend, 0.05 * 255);
  assert(pks.size() >= 2);

  // Plus 1 for second derivative
  size_t offset = 3 * sigma + 1;
  std::pair<size_t, size_t> lines{pks[0].second + offset,
                                  pks[1].second + offset};
  if (lines.first > lines.second) {
    std::swap(lines.first, lines.second);
  }
  return lines;
}

inline bool is_invalid_spot(const Rect &rect, size_t width,
                            size_t height) noexcept {
  constexpr size_t thr = 20;
  return (rect.x1 <= thr) || (rect.x2 + thr >= width - 1) || (rect.y1 <= thr) ||
         (rect.y2 + thr >= height - 1);
}

template <typename ForwardIterator1, typename ForwardIterator2,
          typename ForwardIterator3>
inline std::vector<fp_t>
extract_normalized_spot_data(ForwardIterator1 spot_first,
                             ForwardIterator1 spot_last, ForwardIterator2 bg,
                             ForwardIterator3 mask,
                             std::vector<fp_t> &dest) noexcept {
  std::vector<fp_t> ret;
  while (spot_first != spot_last) {
    auto r = *spot_first;
    auto g = *++spot_first;
    auto b = *++spot_first;
    auto bg_r = *bg;
    auto bg_g = *++bg;
    auto bg_b = *++bg;
    if (*mask == 1) {
      fp_t denom = bg_r * bg_r + bg_g * bg_g + bg_b * bg_b;

      fp_t f = (bg_r * r + bg_g * g + bg_b * b) / denom;
      ret.push_back(f);
      dest.push_back(f);
    }
    ++spot_first;
    ++bg;
    ++mask;
  }
  return ret;
}

template <typename ForwardIterator1, typename ForwardIterator2,
          typename OutputIt>
inline void normalize(ForwardIterator1 first1, ForwardIterator1 last1,
                      ForwardIterator2 first2, OutputIt d_first) noexcept {
  while (first1 != last1) {
    auto bg_r = *first1;
    auto bg_g = *++first1;
    auto bg_b = *++first1;
    fp_t denom = bg_r * bg_r + bg_g * bg_g + bg_b * bg_b;
    auto r = *first2;
    auto g = *++first2;
    auto b = *++first2;
    *d_first = (bg_r * r + bg_g * g + bg_b * b) / denom;
    ++first1;
    ++first2;
    ++d_first;
  }
}

template <typename ForwardIterator>
size_t count_dark_pixels(ForwardIterator first1, ForwardIterator last1,
                         fp_t thresh) noexcept {
  size_t ret = 0;
  while (first1 != last1) {
    if (*first1 <= thresh) {
      ++ret;
    }
    ++first1;
  }
  return ret;
}

template <typename ForwardIterator>
decltype(auto) accumulate(ForwardIterator first, size_t from, size_t to) {
  for (size_t i = 0; i < from; ++i) {
    ++first;
  }
  auto s = *first;
  ++first;
  for (size_t i = from + 1; i < to; ++i) {
    s += *first;
    ++first;
  }
  return s;
}

std::vector<Spot<fp_t>> process(const std::string &path) noexcept {
  std::vector<Spot<fp_t>> spots;
  // Load sample image
  auto sample_file = path + SAMPLE_FOLDER + AVG_FILE_NAME;
  matrix3<uint8_t> im =
      avg_folder<MAX_PICTURE>(path + SAMPLE_FOLDER, AVG_FILE_NAME,
                              Margin{left_off, right_off, top_off, bottom_off});
  if (is_empty(im)) {
    println_e("Couldn't load sample image");
    return spots;
  }

  // Convert RGB image ro YCbCr and extract Y channel information
  auto y_im = rgb2y(im);

  // Solve for front line and origin line
  size_t front, origin;
  std::tie(front, origin) = detect_lines(y_im);

  // Extract the region contains spots
  auto y_spots =
      y_im(slice::all, slice{front + from_front_off, origin - from_origin_off});

  // Load the background image
  auto bg_file = path + BG_FOLDER + AVG_FILE_NAME;
  matrix3<uint8_t> bg = avg_folder<MAX_PICTURE>(
      path + BG_FOLDER, AVG_FILE_NAME,
      Margin{left_off + front + from_front_off,
             right_off + y_im.size(1) - 1 - origin + from_origin_off, top_off,
             bottom_off});

  if (is_empty(bg)) {
    println_e("Couldn't load background image");
    return spots;
  }

  // Convert RGB image ro YCbCr and extract Y channel information
  auto y_bg = rgb2y(bg);

  // Normalize
  auto f = y_spots / y_bg;

  auto h = imhist(f.begin(), f.end(), 256, [](auto x) {
    return std::min(255 * x, static_cast<fp_t>(255));
  });

  auto thresh = pctl_hist_thresh(h, 0.04) / 255;
  auto mask = f <= thresh;

  mask = binary_open(mask, strel_disk(17, 4));

  std::vector<Rect> rects;
  auto labels = bwlabel(mask, rects);

  println_i(sum_all(mask));

  for (size_t i = 0; i < rects.size(); ++i) {
    if (is_invalid_spot(rects[i], f.size(1), f.size(0))) {
      continue;
    }
    auto spot =
        f(slice{rects[i].y1, rects[i].y2}, slice{rects[i].x1, rects[i].x2});
    auto sum1 = sum(spot, 1);
    auto py = fit_array(rects[i].y1, rects[i].y2, sum1, 2);
    auto sum0 = sum(spot, 0);
    auto px = fit_array(rects[i].x1, rects[i].x2, sum0, 2);
    spots.emplace_back(-px(1) / (2 * px(2)), -py(1) / (2 * py(2)), rects[i]);
  }

  std::vector<fp_t> spots_data;
  for (size_t i = 0; i < spots.size(); ++i) {
    auto x1 = spots[i].rect.x1 + front + from_front_off;
    auto x2 = spots[i].rect.x2 + front + from_front_off;
    auto y1 = spots[i].rect.y1;
    auto y2 = spots[i].rect.y2;
    auto spot = im(slice{y1, y2}, slice{x1, x2}, slice::all);
    auto bg_spot = bg(slice{y1, y2}, slice{spots[i].rect.x1, spots[i].rect.x2},
                      slice::all);
    auto mask_spot = mask(slice{spots[i].rect.y1, spots[i].rect.y2},
                          slice{spots[i].rect.x1, spots[i].rect.x2});
    spots[i].data =
        extract_normalized_spot_data(spot.begin(), spot.end(), bg_spot.begin(),
                                     mask_spot.begin(), spots_data);
  }

  h = imhist(spots_data.begin(), spots_data.end(), 256,
             [](auto x) { return std::min(255 * x, static_cast<fp_t>(255)); });

  thresh = pctl_hist_thresh(h, 0.5) / 255;
  println_i(thresh);

  size_t num_pixels = max_allowable_pixels;
  for (size_t i = 0; i < spots.size(); ++i) {
    spots[i].rf = 1 - (spots[i].xc + from_front_off) / (origin - front + 1);
    size_t num_dark_pixels =
        count_dark_pixels(spots[i].data.begin(), spots[i].data.end(), thresh);
    println_i(num_dark_pixels);
    num_pixels =
        std::min(num_pixels, static_cast<size_t>(0.8 * num_dark_pixels));
  }
  println_i(num_pixels);

  for (size_t i = 0; i < spots.size(); ++i) {
    sort(spots[i].data.begin(), spots[i].data.end());
    spots[i].darkness =
        accumulate(spots[i].data.begin(), num_pixels / 4, num_pixels);
  }

  // Sort spots in the order of descending yc
  std::sort(spots.begin(), spots.end(),
            [](auto &a, auto &b) { return a.yc > b.yc; });

  return spots;
}
} // namespace tlc
