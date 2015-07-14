#include <sys/stat.h>
#include "imtoolbox.h"

namespace imtoolbox {

// This is used to specify all elements in a dimension
slice slice::all{};

std::ofstream log;

// Speedup version if predefined offsets are available
matrix<uint8_t, 3> avg_folder(const char *path_name, const char *avg_file_name,
                              size_t num_images,
                              const Offset &offset) noexcept {
  assert(num_images >= 1);
  std::string path{path_name};
  if (path[path.length() - 1] != '/') {
    path += "/";
  }

  std::vector<matrix<uint8_t, 3>> images(num_images);
  for (size_t i = 0; i < num_images; ++i) {
    images[i] = imread<uint8_t, 3>(path + std::to_string(i + 1) + ".jpg");
  }

  size_t height = images[0].size(0);
  size_t width = images[0].size(1);

  std::vector<matrix_ref<uint8_t, 3>> image_rois;
  for (size_t i = 0; i < num_images; ++i) {
    image_rois.emplace_back(
        images[i](slice{offset.top, height - 1 - offset.bottom},
                  slice{offset.left, width - 1 - offset.right}, slice::all));
  }

  // TODO: Check if all images have the same size
  std::vector<decltype(image_rois[0].begin())> pix;
  for (size_t i = 0; i < num_images; ++i) {
    pix.emplace_back(image_rois[i].begin());
  }

  fp_t s = 0;
  for (auto first = image_rois[num_images - 1].begin(),
            last = image_rois[num_images - 1].end();
       first != last; ++first) {
    s = 0;
    for (size_t k = 0; k < num_images; ++k) {
      s += *(pix[k]);
      ++(pix[k]);
    }
    *first = std::round(s / num_images);
  }
  imwrite(image_rois[num_images - 1], path + avg_file_name);
  return image_rois[num_images - 1];
}

matrix<uint8_t, 3> avg_folder(const std::string &path_name,
                              const char *avg_file_name, size_t num_images,
                              const Offset &offset) noexcept {
  return avg_folder(path_name.c_str(), avg_file_name, num_images, offset);
}

bool exist(const char *file_name) noexcept {
  struct stat sb;
  return stat(file_name, &sb) == 0;
}

bool exist(const std::string &file_name) noexcept {
  return exist(file_name.c_str());
}
}
