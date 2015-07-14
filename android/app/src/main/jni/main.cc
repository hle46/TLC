#include "tlc.h"
using namespace imtoolbox;
int main (int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " folder\n";
    return -1;
  }
  begin_log(std::string(argv[1]) + "log.txt");
  std::vector<tlc::Spot<fp_t>> spots = tlc::process(argv[1]);
  for (size_t i = 0; i < spots.size(); ++i) {
    println_i("Spot " + std::to_string(i + 1) + ": ");
    println_i("\tRf: " + std::to_string(spots[i].rf));
    println_i("\tDarkness: " + std::to_string(spots[i].darkness));
  }
  end_log();
  return 0;
}
