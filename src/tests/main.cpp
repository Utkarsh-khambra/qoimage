#include <fmt/core.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../qoimage.hpp"
#include "stb.h"
#include <bit>
#include <fstream>
int main() {
  int x, y, n;
  auto p = stbi_load("/home/utkarsh/qoimage/src/test_files/wikipedia_008.png",
                     &x, &y, &n, 0);
  qoi::Image i;
  std::vector<Pixel> pp;
  Pixel prev(0, 0, 0);
  int l = 5;
  for (auto k = 0; k < y * x * n; k += n)
    pp.emplace_back(p[k], p[k + 1], p[k + 2]);

  stbi_image_free(p);
  auto q = i.encode(pp, x, y);
  std::ofstream f("test.qoi", std::ios_base::binary);
  f.write(reinterpret_cast<char *>(q.data()), q.size());
}
