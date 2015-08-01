#ifndef TLC_H
#define TLC_H
#include <vector>
#include <string>
#include "imtoolbox.h"
namespace tlc {
constexpr size_t left_off = 10;
constexpr size_t right_off = 400;
constexpr size_t top_off = 350;
constexpr size_t bottom_off = 450;
constexpr size_t from_front_off = 100;
constexpr size_t from_origin_off = 100;
constexpr size_t max_allowable_pixels = 400;

constexpr const char *SAMPLE_FOLDER = "sample/";
constexpr const char *BG_FOLDER = "bg/";
constexpr const char *AVG_FILE_NAME = "avg.jpg";
constexpr const char *LOG_FILE = "log.txt";
constexpr size_t MAX_PICTURE = 8;
using namespace imtoolbox;
template <typename T> struct Spot {
  T xc;
  T yc;
  T rf;
  T darkness;
  Rect rect;
  std::vector<T> data;
  Spot(T _xc, T _yc, Rect _rect)
      : xc(_xc), yc(_yc), rf(0), darkness(0), rect(_rect) {}
};
std::vector<Spot<fp_t>> process(const std::string &path) noexcept;
inline std::vector<Spot<fp_t>> process(const char *path) noexcept {
  return process(std::string(path));
}
} // namespace tlc
#endif /* TLC_H */
