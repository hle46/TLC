#ifndef TLC_H
#define TLC_H
#include "imtoolbox.h"
#include <sys/stat.h>

namespace tlc {
using namespace imtoolbox;
constexpr size_t left_off = 30;
constexpr size_t right_off = 400;
constexpr size_t top_off = 350;
constexpr size_t bottom_off = 400;
constexpr size_t from_front_off = 100;
constexpr size_t from_origin_off = 100;

constexpr const char *SAMPLE_FOLDER = "sample/";
constexpr const char *BG_FOLDER = "bg/";
constexpr const char *AVG_FILE_NAME = "avg.png";
constexpr const char *LOG_FILE = "log.txt";
constexpr size_t MAX_PICTURE = 8;

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

template <typename T> struct Spot {
  T xc;
  T yc;
  T rf;
  T darkness;
  Rect rect;
  matrix2<T> data;
  Spot(T _xc, T _yc, Rect _rect)
      : xc(_xc), yc(_yc), rf(0), darkness(0), rect(_rect) {}
};

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

template <typename T, typename ForwardIterator1, typename ForwardIterator2>
void extract_spot_data(ForwardIterator1 first1, ForwardIterator1 last1,
                       ForwardIterator2 first2, std::vector<T> &dest) noexcept {
  while (first1 != last1) {
    if (*first2 == 1) {
      dest.push_back(*first1);
    }
    ++first1;
    ++first2;
  }
}

template <typename ForwardIterator1, typename ForwardIterator2>
size_t count_dark_pixels(ForwardIterator1 first1, ForwardIterator1 last1,
                         ForwardIterator2 first2, fp_t thresh) noexcept {
  size_t ret = 0;
  while (first1 != last1) {
    if (*first2 == 1 && *first1 <= thresh) {
      ++ret;
    }
    ++first1;
    ++first2;
  }
  return ret;
}

std::vector<Spot<fp_t>> process(const std::string &path) noexcept {
  std::vector<Spot<fp_t>> spots;
  // Load sample image
  auto sample_file = path + SAMPLE_FOLDER + AVG_FILE_NAME;
  matrix3<uint8_t> im =
      exist(sample_file)
          ? (imread<uint8_t>(sample_file))
          : avg_folder(path + SAMPLE_FOLDER, AVG_FILE_NAME, MAX_PICTURE,
                       Offset{left_off, right_off, top_off, bottom_off});
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
  matrix3<uint8_t> bg =
      exist(bg_file) ? (imread<uint8_t>(bg_file))
                     : avg_folder(path + BG_FOLDER, AVG_FILE_NAME, MAX_PICTURE,
                                  Offset{left_off + front + from_front_off,
                                         right_off + y_im.size(1) - 1 - origin +
                                             from_origin_off,
                                         top_off, bottom_off});

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
    spots[i].data = matrix2<fp_t>(spot.size(0), spot.size(1));
    normalize(bg_spot.begin(), bg_spot.end(), spot.begin(),
              spots[i].data.begin());
    auto mask_spot = mask(slice{spots[i].rect.y1, spots[i].rect.y2},
                          slice{spots[i].rect.x1, spots[i].rect.x2});
    extract_spot_data(spots[i].data.begin(), spots[i].data.end(),
                      mask_spot.begin(), spots_data);
  }

  h = imhist(spots_data.begin(), spots_data.end(), 256,
             [](auto x) { return std::min(255 * x, static_cast<fp_t>(255)); });

  thresh = pctl_hist_thresh(h, 0.5) / 255;

  for (size_t i = 0; i < spots.size(); ++i) {
    spots[i].rf = 1 - (spots[i].xc + from_front_off) / (origin - front + 1);
    auto mask_spot = mask(slice{spots[i].rect.y1, spots[i].rect.y2},
                          slice{spots[i].rect.x1, spots[i].rect.x2});
    spots[i].darkness = count_dark_pixels(
        spots[i].data.begin(), spots[i].data.end(), mask_spot.begin(), thresh);
  }

  // Sort spots in the order of descending yc
  std::sort(spots.begin(), spots.end(),
            [](auto &a, auto &b) { return a.yc > b.yc; });
  return spots;
}

inline std::vector<Spot<fp_t>> process(const char *path) noexcept {
  return process(std::string(path));
}
} // namespace tlc

#endif /* TLC_H */
