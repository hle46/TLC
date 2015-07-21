#include <sys/stat.h>
#include "imtoolbox.h"

namespace imtoolbox {

// This is used to specify all elements in a dimension
slice slice::all{};

std::ofstream log;

bool exist(const char *file_name) noexcept {
  struct stat sb;
  return stat(file_name, &sb) == 0;
}

bool exist(const std::string &file_name) noexcept {
  return exist(file_name.c_str());
}
}
